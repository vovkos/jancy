//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_McJit.h"
#include "jnc_ct_Parser.llk.h"

#if (_JNC_LLVM_JIT_ORC)
#	include "jnc_ct_OrcJit.h"
#endif

#if (_JNC_LLVM_JIT_LEGACY)
#	include "jnc_ct_LegacyJit.h"
#endif

#if (_AXL_DEBUG)
#	define _JNC_TEST_NO_CODE_GEN 1
#endif

namespace jnc {

//..............................................................................

axl::sl::String*
getTlsStringBuffer() {
	static size_t slot = sys::getTlsMgr()->createSlot();

	sl::String* oldStringBuffer = (sl::String*)sys::getTlsMgr()->getSlotValuePtr(slot);
	if (oldStringBuffer)
		return oldStringBuffer;

	rc::Ptr<sl::String> newStringBuffer = AXL_RC_NEW(rc::Box<sl::String>);
	sys::getTlsMgr()->setSlotValue(slot, newStringBuffer);
	return newStringBuffer;
}

namespace ct {

//..............................................................................

Module::Module():
	m_doxyModule(&m_doxyHost) {
	m_config = g_defaultModuleConfig;
	m_compileFlags = ModuleCompileFlag_StdFlags;
	m_compileState = ModuleCompileState_Idle;
	m_asyncFlags = 0;
	m_tryCompileLevel = 0;
	m_disableAccessCheckLevel = 0;
	m_compileErrorCount = 0;
	m_compileErrorCountLimit = DefaultErrorCountLimit;
	m_compileErrorHandler = NULL;
	m_compileErrorHandlerContext = NULL;
	m_attributeObserver = NULL;
	m_attributeObserverContext = NULL;
	m_attributeObserverItemKindMask = 0;
	m_constructor = NULL;
	m_llvmContext = NULL;
	m_llvmModule = NULL;
	m_jit = NULL;

	finalizeConstruction();
}

Module::~Module() {
	clear();
}

bool
Module::processCompileError(ModuleCompileErrorKind errorKind) {
	if (errorKind >= ModuleCompileErrorKind_PostParse) {
		m_namespaceMgr.closeAllNamespaces();
		m_functionMgr.setCurrentFunction(NULL);
		m_controlFlowMgr.setCurrentBlock(NULL);

		// probably, need more cleanup
	}

	if (m_tryCompileLevel)
		return false;

	if (err::getLastError()->isNoError()) // the error is already processed
		return true;

	if (++m_compileErrorCount > m_compileErrorCountLimit) {
		err::setFormatStringError("%d errors; error limit reached", m_compileErrorCount);
		return false;
	}

	if (m_compileErrorCount == 1) // stop code generation after the very first error
		clearLlvm();

	bool result =
		m_compileErrorHandler &&
		m_compileErrorHandler(m_compileErrorHandlerContext, errorKind);

	if (!result)
		return false;

	err::setError(&err::g_noError);
	return true;
}

void
Module::clear() {
	m_name.clear();
	m_compileArray.clear();
	m_filePathSet.clear();
	m_requireSet.clear();

	m_typeMgr.clear();
	m_attributeMgr.clear();
	m_namespaceMgr.clear();
	m_functionMgr.clear();
	m_variableMgr.clear();
	m_constMgr.clear();
	m_dynamicLayoutMgr.clear();
	m_operatorMgr.clear();
	m_unitMgr.clear();
	m_importMgr.clear();
	m_pragmaMgr.clear();
	m_templateMgr.clear();
	m_extensionLibMgr.clear();
	m_doxyModule.clear();
	m_controlFlowMgr.clear();
	m_gcShadowStackMgr.clear();
	m_codeAssistMgr.clear();

	m_sourceList.clear(); // keep strings alive till the end

	clearLlvm();

	m_constructor = NULL;
	m_config = g_defaultModuleConfig;
	m_compileFlags = ModuleCompileFlag_StdFlags;
	m_compileState = ModuleCompileState_Idle;
	m_compileErrorCount = 0;
	m_tryCompileLevel = 0;
	m_disableAccessCheckLevel = 0;
}

void
Module::clearLlvm() {
	m_llvmIrBuilder.clear();
	m_llvmDiBuilder.clear();

	delete m_jit;
	delete m_llvmModule;
	delete m_llvmContext;

	m_jit = NULL;
	m_llvmModule = NULL;
	m_llvmContext = NULL;

	m_compileFlags &= ~(
		ModuleCompileFlag_DebugInfo |
		ModuleCompileFlag_GcSafePointInPrologue |
		ModuleCompileFlag_GcSafePointInInternalPrologue
	);
}

void
Module::setAsyncFlag(AsyncFlag flag) {
	int32_t flags = m_asyncFlags;
	do {
		flags = sys::atomicCmpXchg(&m_asyncFlags, flags, flags | flag);
	} while (!(flags & flag));
}

void
Module::initialize(
	const sl::StringRef& name,
	const ModuleConfig* config
) {
	clear();

	m_name = name;
	m_config = config ? *config : g_defaultModuleConfig;

	if (!m_config.m_jitKind)
#if (_JNC_LLVM_JIT_LEGACY && _JNC_OS_WIN)
		m_config.m_jitKind = JitKind_Legacy;
#else
		m_config.m_jitKind = JitKind_McJit;
#endif

	m_compileFlags = m_config.m_compileFlags;
	m_compileState = ModuleCompileState_Idle;
	m_compileErrorCount = 0;

#if (_AXL_GCC_ASAN)
	// GC guard page safe points do not work with address sanitizer
	m_compileFlags |= ModuleCompileFlag_SimpleGcSafePoint;
#endif

	if (!(m_compileFlags & ModuleCompileFlag_DisableCodeGen)) {
		m_llvmContext = new llvm::LLVMContext;
#if (LLVM_VERSION_MAJOR >= 15 && LLVM_VERSION_MAJOR < 17)
		m_llvmContext->setOpaquePointers(false); // disable opaque pointer mode
#endif
		m_llvmModule = new llvm::Module("jncModule", *m_llvmContext);
		m_llvmIrBuilder.create();

		if (m_compileFlags & ModuleCompileFlag_DebugInfo)
			m_llvmDiBuilder.create();
	}

	if (!(m_compileFlags & ModuleCompileFlag_StdLibDoc)) {
		m_extensionLibMgr.addStaticLib(jnc_CoreLib_getLib());
		m_extensionLibMgr.addStaticLib(jnc_IntrospectionLib_getLib());
		m_typeMgr.createStdTypes();
		m_variableMgr.createStdVariables();
		m_namespaceMgr.addStdItems();
	}

	m_constMgr.createEmptyLiteralPtr();
}

CodeAssist*
Module::generateCodeAssist(
	jnc_CodeAssistKind kind,
	Module* cacheModule,
	size_t offset,
	const sl::StringRef& source
) {
	m_compileFlags |=
		ModuleCompileFlag_DisableCodeGen |
		ModuleCompileFlag_IgnoreOpaqueClassTypeInfo |
		ModuleCompileFlag_KeepTypedefShadow;

	m_codeAssistMgr.initialize(kind, cacheModule, offset);
	parse("code-assist-source", source);
	parseImports();
	return m_codeAssistMgr.generateCodeAssist();
}

bool
Module::parseImpl(
	ExtensionLib* lib,
	const sl::StringRef& fileName,
	const sl::StringRef& source
) {
	ASSERT(m_compileState < ModuleCompileState_Compiled);

	bool result;

	Unit* unit = m_unitMgr.createUnit(lib, fileName);
	m_unitMgr.setCurrentUnit(unit);

	uint_t flags = LexerFlag_Parse;
	if ((m_compileFlags & ModuleCompileFlag_Documentation) && !lib)
		flags |= LexerFlag_DoxyComments; // also include doxy-comments (but not for libs!)

	Lexer lexer(flags);
	lexer.create(fileName, source);

#if (0)
	for (;;) {
		const Token* token = lexer.getToken();
		switch (token->m_token) {
		case TokenKind_Error:
			printf("lexer error: %s\n", err::getLastErrorDescription().sz());
			return false;

		case TokenKind_Eof:
			printf("EOF\n");
			err::setError("lexer-test-exit");
			return false;
		}

		printf(
			"token(%3d) @%3d: %s\n",
			token->m_tokenKind,
			token->m_pos.m_line + 1,
			sl::StringRef(token->m_pos.m_p, token->m_pos.m_length).sz()
		);

		lexer.nextToken();
	}

	err::setError("lexer-test-exit");
	return false;
#endif

	result = true;
	bool isEof;

	Parser parser(this, NULL, Parser::Mode_Parse);
	parser.create(fileName, Parser::StartSymbol);

	CodeAssistKind codeAssistKind = m_codeAssistMgr.getCodeAssistKind();
	if (!codeAssistKind || !unit->isRootUnit())
		do {
			Token* token = lexer.takeToken();
			token->m_data.m_codeAssistFlags = 0; // tokens can be reused -- ensure 0
			isEof = token->m_token == TokenKind_Eof; // EOF token must be parsed
			result = parser.consumeToken(token);
			if (!result)
				return false;
		} while (!isEof);
	else {
		size_t offset = m_codeAssistMgr.getOffset();
		bool isAfter = false;
		result = true;

		do {
			if (m_asyncFlags & AsyncFlag_CancelCodeAssist)
				return err::fail(err::SystemErrorCode_Cancelled);

			Token* token = lexer.takeToken();
			if (isAfter)
				token->m_data.m_codeAssistFlags = TokenCodeAssistFlag_After;
			else if (calcTokenCodeAssistFlags(token, offset)) {
				if (token->m_tokenKind == TokenKind_Identifier && (token->m_data.m_codeAssistFlags & TokenCodeAssistFlag_At))
					m_codeAssistMgr.prepareIdentifierFallback(*token);

				if (token->m_data.m_codeAssistFlags & TokenCodeAssistFlag_After) {
					m_codeAssistMgr.prepareNamespaceFallback();
					isAfter = true;
				}
			}

			isEof = token->m_token == TokenKind_Eof; // EOF token must be parsed
			if (result)
				result = parser.consumeToken(token);
			else { // keep scanning tokens even after error -- until we get any fallback
				parser.getTokenPool()->put(token);
				if (m_codeAssistMgr.hasFallBack())
					break;
			}
		} while (!isEof);
	}

	m_namespaceMgr.getGlobalNamespace()->getUsingSet()->clear();
	return result;
}

bool
Module::parse(
	const sl::StringRef& fileName,
	const sl::StringRef& source0
) {
	sl::String source = source0;
	m_sourceList.insertTail(source);
	return parseImpl(NULL, fileName, source);
}

bool
Module::parseFile(const sl::StringRef& fileName) {
	ASSERT(m_compileState < ModuleCompileState_Compiled);

	sl::String filePath = io::getFullFilePath(fileName);
#if (_AXL_OS_WIN)
	filePath.makeLowerCase();
#endif
	sl::StringHashTableIterator<bool> it = m_filePathSet.visit(filePath);
	if (it->m_value)
		return true; // already parsed

	io::SimpleMappedFile file;
	bool result = file.open(filePath, io::FileFlag_ReadOnly);
	if (!result)
		return false;

	size_t length = file.getMappingSize();
	sl::String source((const char*)file.p(), length);
	m_sourceList.insertTail(source);
	it->m_value = true;
	return parseImpl(NULL, filePath, source);
}

bool
Module::parseImports() {
	ASSERT(m_compileState < ModuleCompileState_Compiled);

	bool finalResult = true;

	for (;;) {
		sl::List<Import> importList;
		m_importMgr.takeOverImports(&importList);
		if (importList.isEmpty())
			break;

		sl::ConstIterator<Import> importIt = importList.getHead();
		for (; importIt; importIt++) {
			bool result = importIt->m_importKind == ImportKind_Source ?
				parseImpl(
					importIt->m_lib,
					importIt->m_filePath,
					importIt->m_source
				) :
				parseFile(importIt->m_filePath);

			if (!result)
				finalResult = false;
		}
	}

	m_compileState = ModuleCompileState_Parsed;
	return finalResult;
}

bool
Module::compile() {
	bool result = compileImpl();
	m_extensionLibMgr.closeDynamicLibZipReaders();
	return result;
}

bool
Module::compileImpl() {
	bool result;

	ASSERT(m_compileState < ModuleCompileState_Compiled);
	if (m_compileState < ModuleCompileState_Parsed) {
		result = parseImports();
		if (!result)
			return false;
	}

	result =
		m_namespaceMgr.getGlobalNamespace()->resolveOrphans() &&
		m_variableMgr.allocateNamespaceVariables(sl::ConstIterator<Variable>()) &&
		m_functionMgr.finalizeNamespaceProperties(sl::ConstIterator<Property>());

	if (!result)
		return false;

	do { // creating opaque class might require more items, hence a loop
		result =
			processRequireSet() &&
			processCompileArray();

		if (!result)
			return false;
	} while (!m_requireSet.isEmpty());

	if (m_compileErrorCount) {
		err::setFormatStringError("%d error(s); compilation failed", m_compileErrorCount);
		return false;
	}

	if (hasCodeGen()) {
		createConstructor();

		result = m_variableMgr.createTlsStructType();
		if (!result)
			return false;

		m_functionMgr.replaceFieldVariableAllocas();
		m_functionMgr.replaceAsyncAllocas(); // after replacing field-variable allocas!
		m_controlFlowMgr.deleteUnreachableBlocks();
	}

	if (m_compileFlags & ModuleCompileFlag_DebugInfo)
		m_llvmDiBuilder.finalize();

	m_compileState = ModuleCompileState_Compiled;
	return true;
}

bool
Module::optimize(uint_t level) {
	// optimization requires knowledge of DataLayout for the TargetMachine
	// which in turn is selected by a particular JIT engine

	if (!m_jit) {
		bool result = createJit();
		if (!result)
			return false;
	}

	// before running LLVM optimization passes, save LLVM names of declarations --
	// corresponding llvm::Function*/llvm::GlobalVariable* may be optimized out

	// things would be much easier if we could just derive from llvm::Function
	// and override eraseFromParent -- then we could update jnc::ct::Function when
	// llvm::Function is optimized out

	// alas, we can't -- the constructor of llvm::Function is private

	sl::Iterator<Function> it = m_functionMgr.m_functionList.getHead();
	for (; it; it++) {
		if (!it->hasLlvmFunction())
			continue;

		llvm::Function* llvmFunction = it->getLlvmFunction();
		if (llvmFunction->isDeclaration())
			it->m_llvmFunctionName = llvmFunction->getName() >> toAxl;
	}

	size_t count = m_variableMgr.m_staticVariableArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Variable* variable = m_variableMgr.m_staticVariableArray[i];
		llvm::GlobalVariable* llvmGlobalVariable = variable->getLlvmGlobalVariable();
		if (llvmGlobalVariable->isDeclaration())
			variable->m_llvmGlobalVariableName = llvmGlobalVariable->getName() >> toAxl;
	}

#if (LLVM_VERSION_MAJOR < 17)
	llvm::PassManagerBuilder passManagerBuilder;
	passManagerBuilder.OptLevel = level;
	passManagerBuilder.SizeLevel = 0;
	passManagerBuilder.Inliner = llvm::createFunctionInliningPass();

	llvm::legacy::FunctionPassManager llvmFunctionPassMgr(m_llvmModule);
	llvm::legacy::PassManager llvmModulePassMgr;
	passManagerBuilder.populateFunctionPassManager(llvmFunctionPassMgr);
	passManagerBuilder.populateModulePassManager(llvmModulePassMgr);

	llvmFunctionPassMgr.doInitialization();

	it = m_functionMgr.m_functionList.getHead();
	for (; it; it++)
		if (!it->isEmpty()) {
			llvm::Function* llvmFunction = it->getLlvmFunction();
			llvmFunctionPassMgr.run(*llvmFunction);
		}

	llvmFunctionPassMgr.doFinalization();

	llvmModulePassMgr.run(*m_llvmModule);
#else
	llvm::LoopAnalysisManager llvmLoopAnalysisMgr;
	llvm::FunctionAnalysisManager llvmFuncAnalysisMgr;
	llvm::CGSCCAnalysisManager llvmCallGraphAnalysisMgr;
	llvm::ModuleAnalysisManager llvmModuleAnalysisMgr;

	llvm::PassBuilder llvmPassBuilder;
	llvmPassBuilder.registerModuleAnalyses(llvmModuleAnalysisMgr);
	llvmPassBuilder.registerCGSCCAnalyses(llvmCallGraphAnalysisMgr);
	llvmPassBuilder.registerFunctionAnalyses(llvmFuncAnalysisMgr);
	llvmPassBuilder.registerLoopAnalyses(llvmLoopAnalysisMgr);
	llvmPassBuilder.crossRegisterProxies(
		llvmLoopAnalysisMgr,
		llvmFuncAnalysisMgr,
		llvmCallGraphAnalysisMgr,
		llvmModuleAnalysisMgr
	);

	static llvm::OptimizationLevel llvmOptLevelTable[] = {
		llvm::OptimizationLevel::O0,
		llvm::OptimizationLevel::O1,
		llvm::OptimizationLevel::O2,
		llvm::OptimizationLevel::O3,
	};

	if (level >= countof(llvmOptLevelTable))
		level = countof(llvmOptLevelTable) - 1;

	llvm::ModulePassManager llvmModulePassMgr = llvmPassBuilder.buildPerModuleDefaultPipeline(llvmOptLevelTable[level]);
	llvmModulePassMgr.run(*m_llvmModule, llvmModuleAnalysisMgr);
#endif
	return true;
}

bool
Module::createJit() {
	ASSERT(!m_jit);

	switch (m_config.m_jitKind) {
	case JitKind_McJit:
		m_jit = new McJit(this);
		break;

#if (_JNC_LLVM_JIT_ORC)
	case JitKind_Orc:
		m_jit = new OrcJit(this);
		break;
#endif

#if (_JNC_LLVM_JIT_LEGACY)
	case JitKind_Legacy:
		m_jit = new LegacyJit(this);
		break;
#endif

	default:
		err::setFormatStringError("Invalid JIT engine kind: %d", m_config.m_jitKind);
		return false;
	}

	ASSERT(m_jit);
	bool result = m_jit->create(m_config.m_jitOptLevel);
	if (!result) {
		clearLlvm(); // JIT takes ownership of the module, so we have to clear all LLVM stuff anyway
		return false;
	}

	return true;
}

bool
Module::jit() {
	bool result;

	ASSERT(m_compileState < ModuleCompileState_Jitted);
	if (m_compileState < ModuleCompileState_Compiled) {
		result = compile();
		if (!result)
			return false;
	}

	if (!m_jit) {
		result = createJit();
		if (!result)
			return false;
	}

	result =
		m_extensionLibMgr.mapAddresses() &&
		m_jit->prepare() &&
		m_functionMgr.jitFunctions();

	if (!result) {
		delete m_jit; // JIT throws and gets into an inconsistent state; it's best to delete it immediately
		m_jit = NULL;
		return false;
	}

	m_compileState = ModuleCompileState_Jitted;
	return true;
}

bool
Module::requireIntrospectionLib() {
	ASSERT(!(m_compileFlags & AuxCompileFlag_IntrospectionLib));

	static StdType introspectionTypeTable[] = {
		StdType_ModuleItem,
		StdType_ModuleItemDecl,
		StdType_ModuleItemInitializer,
		StdType_Attribute,
		StdType_AttributeBlock,
		StdType_Namespace,
		StdType_GlobalNamespace,
		StdType_Type,
		StdType_DataPtrType,
		StdType_NamedType,
		StdType_MemberBlock,
		StdType_BaseTypeSlot,
		StdType_DerivableType,
		StdType_ArrayType,
		StdType_FunctionArg,
		StdType_FunctionType,
		StdType_FunctionPtrType,
		StdType_PropertyType,
		StdType_PropertyPtrType,
		StdType_EnumConst,
		StdType_EnumType,
		StdType_ClassType,
		StdType_ClassPtrType,
		StdType_Field,
		StdType_StructType,
		StdType_UnionType,
		StdType_Alias,
		StdType_Const,
		StdType_Variable,
		StdType_Function,
		StdType_FunctionOverload,
		StdType_Property,
		StdType_Typedef,
		StdType_Module,
		StdType_Unit,
	};

	for (size_t i = 0; i < countof(introspectionTypeTable); i++) {
		bool result = m_typeMgr.getStdType(introspectionTypeTable[i])->require();
		if (!result)
			return false;
	}

	m_compileFlags |= AuxCompileFlag_IntrospectionLib;
	return true;
}

bool
Module::requireDynamicLayout() {
	ASSERT(!(m_compileFlags & AuxCompileFlag_DynamicLayout));
	bool result =
		ensureIntrospectionLibRequired() &&
		m_typeMgr.getStdType(StdType_DynamicLayout)->require() &&
		m_typeMgr.getStdType(StdType_DynamicSection)->require() &&
		m_typeMgr.getStdType(StdType_Promise)->require();

	if (!result)
		return false;

	m_compileFlags |= AuxCompileFlag_DynamicLayout;
	return true;
}

bool
Module::processRequireSet() {
	bool result;

	sl::StringHashTableIterator<RequiredItem> requireIt = m_requireSet.getHead();
	for (; requireIt; requireIt++) {
		FindModuleItemResult findResult = jnc_g_nullFindModuleItemResult;
		if (!(requireIt->m_value.m_flags & ModuleRequireFlag_Traverse))
			findResult = m_namespaceMgr.getGlobalNamespace()->findItem(requireIt->getKey());
		else { // traverse at each step (not the same as Namespace::findItemTraverse)
			QualifiedName name;
			name.parse(requireIt->getKey());

			Namespace* nspace = m_namespaceMgr.getGlobalNamespace();
			findResult = nspace->findDirectChildItem(name.getFirstName());
			if (findResult.m_item) {
				sl::ConstBoxIterator<QualifiedNameAtom> nameIt = name.getNameList().getHead();
				for (; nameIt; nameIt++) {
					Namespace* nspace = findResult.m_item->getNamespace();
					if (!nspace) {
						findResult = g_nullFindModuleItemResult;
						break;
					}

					if (nameIt->m_atomKind != QualifiedNameAtomKind_Name) {
						findResult = g_nullFindModuleItemResult;
						break;
					}

					findResult = nspace->findDirectChildItemTraverse(nameIt->m_name);
					if (!findResult.m_item)
						break;
				}
			}
		}

		if (!findResult.m_result)
			return false;

		if (!findResult.m_item) {
			if (!(requireIt->m_value.m_flags & ModuleRequireFlag_Essential)) // not essential
				continue;

			err::setFormatStringError("required module item '%s' not found", requireIt->getKey().sz());
			return false;
		}

		if (requireIt->m_value.m_itemKind != ModuleItemKind_Undefined) {
			if (findResult.m_item->getItemKind() != requireIt->m_value.m_itemKind) {
				err::setFormatStringError(
					"required module item '%s' item kind mismatch: %s vs %s",
					requireIt->getKey().sz(),
					getModuleItemKindString(requireIt->m_value.m_itemKind),
					getModuleItemKindString(findResult.m_item->getItemKind())
				);
				return false;
			}

			if (requireIt->m_value.m_itemKind == ModuleItemKind_Type &&
				requireIt->m_value.m_typeKind != TypeKind_Void &&
				requireIt->m_value.m_typeKind != ((Type*)findResult.m_item)->getTypeKind()) {
				err::setFormatStringError(
					"required type '%s' type mismatch: '%s'",
					requireIt->getKey().sz(),
					((Type*)findResult.m_item)->getTypeString().sz()
				);
				return false;
			}
		}

		if (!(requireIt->m_value.m_flags & ModuleRequireFlag_Essential) &&
			findResult.m_item->getItemKind() == ModuleItemKind_Function &&
			((Function*)findResult.m_item)->isPrototype())
			continue; // ignore unimplemented prototypes

		result = findResult.m_item->require();
		if (!result)
			return false;
	}

	m_requireSet.clear();
	return true;
}

bool
Module::processCompileArray() {
	bool result;

	// new items could be added in the process, so we need a loop

	while (!m_compileArray.isEmpty()) {
		sl::Array<Function*> compileArray;
		sl::takeOver(&compileArray, &m_compileArray);

		size_t count = compileArray.getCount();
		for (size_t i = 0; i < compileArray.getCount(); i++) {
			Function* function = compileArray[i];
			result =
				function->ensureAttributeValuesReady() &&
				function->compile();

			if (!result) {
				lex::ensureSrcPosError(
					function->m_parentUnit ? function->m_parentUnit->getFilePath() : m_name,
					function->m_pos
				);

				result = processCompileError(ModuleCompileErrorKind_PostParse);
				if (!result)
					return false;

				m_namespaceMgr.closeAllNamespaces();
			}

			ASSERT(!m_namespaceMgr.getCurrentScope());
		}

		if (hasCodeGen() && !m_variableMgr.getGlobalVariablePrimeArray().isEmpty()) {
			Function* function = createGlobalPrimerFunction();
			m_functionMgr.addGlobalCtorDtor(GlobalCtorDtorKind_VariablePrimer, function);
		}

		if (!m_variableMgr.getGlobalVariableInitializeArray().isEmpty()) {
			Function* function = createGlobalInitializerFunction();
			if (!function) {
				result = processCompileError(ModuleCompileErrorKind_PostParse);
				if (!result)
					return false;
			} else {
				m_functionMgr.addGlobalCtorDtor(GlobalCtorDtorKind_VariableInitializer, function);
			}
		}

		result = m_typeMgr.requireExternalReturnTypes();
		if (!result) {
			result = processCompileError(ModuleCompileErrorKind_PostParse);
			if (!result)
				return false;
		}
	}

	return true;
}

void
Module::createConstructor() {
	ASSERT(!m_constructor);

	const sl::Array<Variable*>& staticArray = m_variableMgr.getStaticVariableArray();
	const sl::Array<Function*>& primerArray = m_functionMgr.getGlobalCtorDtorArray(GlobalCtorDtorKind_VariablePrimer);
	const sl::Array<Function*>& initializerArray = m_functionMgr.getGlobalCtorDtorArray(GlobalCtorDtorKind_VariableInitializer);
	const sl::Array<Function*>& constructorArray = m_functionMgr.getGlobalCtorDtorArray(GlobalCtorDtorKind_Constructor);
	const sl::Array<Function*>& destructorArray = m_functionMgr.getGlobalCtorDtorArray(GlobalCtorDtorKind_Destructor);

	if (staticArray.isEmpty() &&
		primerArray.isEmpty() &&
		initializerArray.isEmpty() &&
		constructorArray.isEmpty() &&
		destructorArray.isEmpty())
		return;

	FunctionType* constructorType = (FunctionType*)m_typeMgr.getStdType(StdType_SimpleFunction);
	m_constructor = m_functionMgr.createInternalFunction("module.construct", constructorType);
	m_constructor->m_storageKind = StorageKind_Static;

	uint_t prevFlags = m_compileFlags;
	m_compileFlags &= ~ModuleCompileFlag_GcSafePointInInternalPrologue;
	m_functionMgr.internalPrologue(m_constructor);
	m_compileFlags = prevFlags;

	size_t count = staticArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Variable* variable = staticArray[i];
		if (variable->getStdVariable() != StdVariable_GcSafePointTrigger &&
			variable->getType()->getTypeKind() != TypeKind_Class &&  // classes are on primerArray anyway
			!(variable->getPtrTypeFlags() & PtrTypeFlag_Const)) // const variables are either initialized or mapped (external)
			m_operatorMgr.zeroInitialize(variable);
	}

	count = primerArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_llvmIrBuilder.createCall(primerArray[i], primerArray[i]->getType(), NULL);

	count = initializerArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_llvmIrBuilder.createCall(initializerArray[i], initializerArray[i]->getType(), NULL);

	count = constructorArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_llvmIrBuilder.createCall(constructorArray[i], constructorArray[i]->getType(), NULL);

	count = destructorArray.getCount();
	if (count) {
		Function* addStaticDestructor = m_functionMgr.getStdFunction(StdFunc_AddStaticDestructor);
		Type* voidPtrType = m_typeMgr.getStdType(StdType_ByteThinPtr);

		for (size_t i = 0; i < count; i++) {
			Value argValue;
			m_llvmIrBuilder.createBitCast(destructorArray[i], voidPtrType, &argValue);
			m_llvmIrBuilder.createCall(addStaticDestructor, addStaticDestructor->getType(), argValue, NULL);
		}
	}

	m_functionMgr.internalEpilogue();
}

Function*
Module::createGlobalPrimerFunction() {
	FunctionType* type = (FunctionType*)m_typeMgr.getStdType(StdType_SimpleFunction);
	Function* function = m_functionMgr.createInternalFunction("module.primeGlobals", type);
	function->m_storageKind = StorageKind_Static;

	m_functionMgr.internalPrologue(function);
	m_variableMgr.primeGlobalVariables();
	m_functionMgr.internalEpilogue();
	return function;
}

Function*
Module::createGlobalInitializerFunction() {
	FunctionType* type = (FunctionType*)m_typeMgr.getStdType(StdType_SimpleFunction);
	Function* function = m_functionMgr.createInternalFunction("module.initializeGlobals", type);
	function->m_storageKind = StorageKind_Static;

	m_functionMgr.internalPrologue(function);

	bool result = m_variableMgr.initializeGlobalVariables();
	if (!result)
		return NULL;

	m_functionMgr.internalEpilogue();
	return function;
}

sl::String
Module::getLlvmIrString() {
	std::string string;
	llvm::raw_string_ostream stream(string);
	m_llvmModule->print(stream, NULL);
	stream.flush();
	return sl::String(string.data(), string.length());
}

//..............................................................................

} // namespace ct
} // namespace jnc
