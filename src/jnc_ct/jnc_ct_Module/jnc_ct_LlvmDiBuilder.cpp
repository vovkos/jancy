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
#include "jnc_ct_LlvmDiBuilder.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_UnionType.h"

namespace jnc {
namespace ct {

//..............................................................................

LlvmDiBuilder::LlvmDiBuilder() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_llvmDiBuilder = NULL;
}

void
LlvmDiBuilder::create() {
	clear();

	llvm::Module* llvmModule = m_module->getLlvmModule();
	ASSERT(llvmModule);

	m_llvmDiBuilder = new llvm::DIBuilder(*llvmModule);

	m_llvmDiBuilder->createCompileUnit(
		llvm::dwarf::DW_LANG_C99,
#if (LLVM_VERSION < 0x040000)
		m_module->getName().sz(),
		io::getCurrentDir().sz(),
#else
		createFile(m_module->getName(), io::getCurrentDir()),
#endif
		"jnc-1.0.0",
		false, "", 1
	);
}

llvm::DebugLoc
LlvmDiBuilder::getDebugLoc(
	Scope* scope,
	const lex::LineCol& pos
) {
	llvm::DIScope_vn llvmDiScope = scope->getLlvmDiScope();
	if (!llvmDiScope)
		llvmDiScope = m_module->m_functionMgr.getCurrentFunction()->getLlvmDiSubprogram();

#if (LLVM_VERSION_MAJOR < 12)
	return llvm::DebugLoc::get(pos.m_line + 1, pos.m_col + 1, llvmDiScope);
#else
	return llvm::DILocation::get(*m_module->getLlvmContext(), pos.m_line + 1, pos.m_col + 1, (llvm::MDNode*)llvmDiScope);
#endif
}

llvm::DebugLoc
LlvmDiBuilder::getEmptyDebugLoc() {
	// llvm magic: unfortunately, simple llvm::DebugLoc() doesn't quit cut it

	Unit* unit = m_module->m_unitMgr.getCurrentUnit();

#if (LLVM_VERSION_MAJOR < 12)
	return unit ?
		llvm::DebugLoc::get(0, 0, unit->getLlvmDiFile()) :
		llvm::DebugLoc();
#else
	ASSERT((llvm::MDNode*)unit->getLlvmDiFile());

	return unit ?
		llvm::DebugLoc(llvm::DILocation::get(*m_module->getLlvmContext(), 0, 0, (llvm::MDNode*)unit->getLlvmDiFile())) :
		llvm::DebugLoc();
#endif
}

llvm::DISubroutineType_vn
LlvmDiBuilder::createSubroutineType(FunctionType* functionType) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	sl::Array<FunctionArg*> argArray = functionType->getArgArray();
	size_t count = argArray.getCount();

	char buffer[256];
	sl::Array<llvm::Metadata*> argTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	argTypeArray.setCount(count + 1);

	llvm::Metadata** dst = argTypeArray.p();
	*dst = functionType->getReturnType()->getLlvmDiType();
	dst++;

	for (size_t i = 0; i < count; i++, dst++)
		*dst = argArray[i]->getType()->getLlvmDiType();

#if (LLVM_VERSION < 0x030600)
	llvm::DIArray llvmDiTypeArray = m_llvmDiBuilder->getOrCreateArray(llvm::ArrayRef<llvm::Metadata*> (argTypeArray, count + 1));
	return m_llvmDiBuilder->createSubroutineType(unit->getLlvmDiFile(), llvmDiTypeArray);
#elif (LLVM_VERSION < 0x030700)
	llvm::DITypeArray llvmDiTypeArray = m_llvmDiBuilder->getOrCreateTypeArray(llvm::ArrayRef<llvm::Metadata*> (argTypeArray, count + 1));
	return m_llvmDiBuilder->createSubroutineType(unit->getLlvmDiFile(), llvmDiTypeArray);
#elif (LLVM_VERSION < 0x030800)
	llvm::DITypeRefArray llvmDiTypeArray = m_llvmDiBuilder->getOrCreateTypeArray(llvm::ArrayRef<llvm::Metadata*> (argTypeArray, count + 1));
	return m_llvmDiBuilder->createSubroutineType(unit->getLlvmDiFile(), llvmDiTypeArray);
#else
	llvm::DITypeRefArray llvmDiTypeArray = m_llvmDiBuilder->getOrCreateTypeArray(llvm::ArrayRef<llvm::Metadata*> (argTypeArray, count + 1));
	return m_llvmDiBuilder->createSubroutineType(llvmDiTypeArray);
#endif
}

llvm::DIType_vn
LlvmDiBuilder::createEmptyStructType(StructType* structType) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	return m_llvmDiBuilder->createStructType(
		unit->getLlvmDiFile(),
		structType->getQualifiedName().sz(),
		unit->getLlvmDiFile(),
		structType->getPos().m_line + 1,
		structType->getSize() * 8,
		structType->getAlignment() * 8,
		(llvm::DIFlags) 0,
#if (LLVM_VERSION < 0x030700)
		llvm::DIType(),     // derived from
#else
		NULL,                // derived from
#endif
		llvm::DINodeArray() // elements -- set body later
	);
}

void
LlvmDiBuilder::setStructTypeBody(StructType* structType) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	sl::ConstList<BaseTypeSlot> baseTypeList = structType->getBaseTypeList();
	const sl::Array<Field*>& fieldArray = structType->getFieldArray();

	size_t baseTypeCount = baseTypeList.getCount();
	size_t fieldCount = fieldArray.getCount();
	size_t count = baseTypeCount + fieldCount;

	char buffer[256];
	sl::Array<llvm::Metadata*> fieldTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	fieldTypeArray.setCount(count);
	sl::Array<llvm::Metadata*>::Rwi rwi = fieldTypeArray;

	size_t i = 0;

	sl::ConstIterator<BaseTypeSlot> baseTypeIt = baseTypeList.getHead();
	for (; baseTypeIt; i++, baseTypeIt++) {
		const BaseTypeSlot* baseType = *baseTypeIt;
		sl::String name = baseType->getType()->getQualifiedName();

		rwi[i] = m_llvmDiBuilder->createMemberType(
			unit->getLlvmDiFile(),
			!name.isEmpty() ? name.sz() : "UnnamedBaseType",
			unit->getLlvmDiFile(),
			baseType->getPos().m_line + 1,
			baseType->getType()->getSize() * 8,
			baseType->getType()->getAlignment() * 8,
			baseType->getOffset() * 8,
			(llvm::DIFlags) 0,
			baseType->getType()->getLlvmDiType()
		);
	}

	for (size_t j = 0; j < fieldCount; i++, j++) {
		Field* field = fieldArray[j];
		sl::String name = field->getName();

		rwi[i] = m_llvmDiBuilder->createMemberType(
			unit->getLlvmDiFile(),
			!name.isEmpty() ? name.sz() : "m_unnamedField",
			unit->getLlvmDiFile(),
			field->getPos().m_line + 1,
			field->getType()->getSize() * 8,
			field->getType()->getAlignment() * 8,
			field->getOffset() * 8,
			(llvm::DIFlags) 0,
			field->getType()->getLlvmDiType()
		);
	}

	llvm::DINodeArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray(llvm::ArrayRef<llvm::Metadata*> (fieldTypeArray, count));

#if (LLVM_VERSION < 0x030600)
	llvm::DICompositeType llvmDiType(structType->getLlvmDiType());
	ASSERT(llvmDiType);
	llvmDiType.setTypeArray(llvmDiArray);
#elif (LLVM_VERSION < 0x030700)
	llvm::DICompositeType llvmDiType(structType->getLlvmDiType());
	ASSERT(llvmDiType);
	llvmDiType->replaceOperandWith(4, llvmDiArray);
#else
	llvm::DICompositeType* llvmDiType = (llvm::DICompositeType*)structType->getLlvmDiType();
	ASSERT(llvm::isa<llvm::DICompositeType> (llvmDiType));
	llvmDiType->replaceElements(llvmDiArray);
#endif
}

llvm::DIType_vn
LlvmDiBuilder::createEmptyUnionType(UnionType* unionType) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	return m_llvmDiBuilder->createUnionType(
		unit->getLlvmDiFile(),
		unionType->getQualifiedName().sz(),
		unit->getLlvmDiFile(),
		unionType->getPos().m_line + 1,
		unionType->getSize() * 8,
		unionType->getAlignment() * 8,
		(llvm::DIFlags) 0,
		llvm::DINodeArray() // elements -- set body later
	);
}

void
LlvmDiBuilder::setUnionTypeBody(UnionType* unionType) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	const sl::Array<Field*>& fieldArray = unionType->getFieldArray();
	size_t count = fieldArray.getCount();

	char buffer[256];
	sl::Array<llvm::Metadata*> fieldTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	fieldTypeArray.setCount(count);
	sl::Array<llvm::Metadata*>::Rwi rwi = fieldTypeArray;

	for (size_t i = 0; i < count; i++) {
		Field* field = fieldArray[i];
		sl::String name = field->getName();

		rwi[i] = m_llvmDiBuilder->createMemberType(
			unit->getLlvmDiFile(),
			!name.isEmpty() ? name.sz() : "m_unnamedField",
			unit->getLlvmDiFile(),
			field->getPos().m_line + 1,
			field->getType()->getSize() * 8,
			field->getType()->getAlignment() * 8,
			field->getOffset() * 8,
			(llvm::DIFlags) 0,
			field->getType()->getLlvmDiType()
		);
	}

	llvm::DINodeArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray(llvm::ArrayRef<llvm::Metadata*> (fieldTypeArray, count));

#if (LLVM_VERSION < 0x030600)
	llvm::DICompositeType llvmDiType(unionType->getLlvmDiType());
	ASSERT(llvmDiType);
	llvmDiType.setTypeArray(llvmDiArray);
#elif (LLVM_VERSION < 0x030700)
	llvm::DICompositeType llvmDiType(unionType->getLlvmDiType());
	ASSERT(llvmDiType);
	llvmDiType->replaceOperandWith(4, llvmDiArray);
#else
	llvm::DICompositeType* llvmDiType = (llvm::DICompositeType*)unionType->getLlvmDiType();
	ASSERT(llvm::isa<llvm::DICompositeType> (llvmDiType));
	llvmDiType->replaceElements(llvmDiArray);
#endif
}

llvm::DIType_vn
LlvmDiBuilder::createArrayType(ArrayType* arrayType) {
	char buffer[256];
	sl::Array<llvm::Metadata*> dimArray(rc::BufKind_Stack, buffer, sizeof(buffer));

	ArrayType* p = arrayType;
	for (;;) {
		Type* elementType = p->getElementType();
		size_t elementCount = p->getElementCount();
		ASSERT(elementCount);

		AXL_TODO("seems like a bug in LLVM DiBuilder (should be ElementCount - 1)")
		dimArray.append(m_llvmDiBuilder->getOrCreateSubrange(0, elementCount));

		if (elementType->getTypeKind() != TypeKind_Array)
			break;

		p = (ArrayType*)elementType;
	}

	llvm::DINodeArray llvmDiArray = m_llvmDiBuilder->getOrCreateArray(llvm::ArrayRef<llvm::Metadata*> (dimArray, dimArray.getCount()));

	return m_llvmDiBuilder->createArrayType(
		arrayType->getSize() * 8,
		arrayType->getAlignment() * 8,
		arrayType->getRootType()->getLlvmDiType(),
		llvmDiArray
	);
}

llvm::DIType_vn
LlvmDiBuilder::createPointerType(Type* type) {
	return m_llvmDiBuilder->createPointerType(
		type->getLlvmDiType(),
		type->getSize() * 8,
		type->getAlignment() * 8
	);
}

#if (LLVM_VERSION < 0x040000)

llvm::DIGlobalVariable_vn
LlvmDiBuilder::createGlobalVariable(Variable* variable) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	llvm::GlobalVariable* llvmGlobalVariable = (llvm::GlobalVariable*)variable->getLlvmValue();
	ASSERT(llvm::isa<llvm::GlobalVariable> (llvmGlobalVariable));

	return m_llvmDiBuilder->createGlobalVariable(
#if (LLVM_VERSION < 0x030600)
		// no context argument
#elif (LLVM_VERSION < 0x030700)
		llvm::DIDescriptor(), // DIDescriptor Context
#else
		NULL, // DIScope* Context
#endif
		variable->getQualifiedName().sz(), // StringRef Name
		variable->getQualifiedName().sz(), // StringRef LinkageName
		unit->getLlvmDiFile(),
		variable->getPos().m_line + 1,
		variable->getType()->getLlvmDiType(),
		true, // bool isLocalToUnit
		llvmGlobalVariable
	);
}

#endif

llvm::DIVariable_vn
LlvmDiBuilder::createParameterVariable(
	Variable* variable,
	size_t argumentIdx
) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	ASSERT(unit && scope);

#if (LLVM_VERSION < 0x030800)
	return m_llvmDiBuilder->createLocalVariable(
		llvm::dwarf::DW_TAG_arg_variable,
		scope->getLlvmDiScope(),
		variable->getName().sz(),
		unit->getLlvmDiFile(),
		variable->getPos().m_line + 1,
		variable->getType()->getLlvmDiType(),
		true, // bool AlwaysPreserve
		0     // unsigned Flags
	);
#else
	return m_llvmDiBuilder->createParameterVariable(
		scope->getLlvmDiScope(),
		variable->getName().sz(),
		argumentIdx + 1,
		unit->getLlvmDiFile(),
		variable->getPos().m_line + 1,
		variable->getType()->getLlvmDiType(),
		true, // bool AlwaysPreserve
		(llvm::DIFlags) 0
	);
#endif
}

llvm::Instruction*
LlvmDiBuilder::createDeclare(Variable* variable) {
	BasicBlock* block = m_module->m_controlFlowMgr.getCurrentBlock();
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();

	ASSERT(block && scope);

#if (LLVM_VERSION < 0x030700)
	llvm::Instruction* llvmInstruction = m_llvmDiBuilder->insertDeclare(
		variable->getLlvmValue(),
		variable->getLlvmDiDescriptor(),
#	if (LLVM_VERSION >= 0x030600)
		m_llvmDiBuilder->createExpression(),
#	endif
		block->getLlvmBlock()
	);

	llvm::DebugLoc llvmDebugLoc = llvm::DebugLoc::get(
		variable->getPos().m_line + 1, 0,
		scope->getLlvmDiScope()
	);

	llvmInstruction->setDebugLoc(llvmDebugLoc);
#else
	llvm::DILocalVariable* llvmDiLocalVariable = (llvm::DILocalVariable*)variable->getLlvmDiDescriptor();
	ASSERT(llvm::isa<llvm::DILocalVariable> (llvmDiLocalVariable));
	ASSERT((llvm::MDNode*)scope->getLlvmDiScope());

#if (LLVM_VERSION_MAJOR < 12)
	llvm::DebugLoc llvmDebugLoc = llvm::DebugLoc::get(variable->getPos().m_line, variable->getPos().m_col, scope->getLlvmDiScope());
#	else
	llvm::DebugLoc llvmDebugLoc = llvm::DILocation::get(
		*m_module->getLlvmContext(),
		variable->getPos().m_line,
		variable->getPos().m_col,
		(llvm::MDNode*)scope->getLlvmDiScope()
	);
#	endif

	llvm::Instruction* llvmInstruction = m_llvmDiBuilder->insertDeclare(
		variable->getLlvmValue(),
		llvmDiLocalVariable,
		m_llvmDiBuilder->createExpression(),
		llvmDebugLoc,
		block->getLlvmBlock()
	);
#endif

	return llvmInstruction;
}

llvm::DISubprogram_vn
LlvmDiBuilder::createFunction(Function* function) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	lex::LineCol declPos = function->getPos();
	lex::LineCol scopePos = function->hasBody() ? (const lex::LineCol&)function->getBodyPos() : declPos;

#if (LLVM_VERSION < 0x030700)
	llvm::DICompositeType llvmDiSubroutineType(function->getType()->getLlvmDiType());
	ASSERT(llvmDiSubroutineType);

	return m_llvmDiBuilder->createFunction(
		unit->getLlvmDiFile(),             // DIDescriptor Scope
		function->getQualifiedName().sz(), // StringRef Name
		function->getQualifiedName().sz(), // StringRef LinkageName
		unit->getLlvmDiFile(),             // DIFile File
		declPos.m_line + 1,                // unsigned LineNo
		llvmDiSubroutineType,              // DICompositeType Ty
		false,                             // bool isLocalToUnit
		true,                              // bool isDefinition
		scopePos.m_line + 1,               // unsigned ScopeLine
		0,                                 // llvm::DIDescriptor::FlagPrototyped,
		false,                             // bool isOptimized
		function->getLlvmFunction(),       // Function *Fn
		NULL,                              // MDNode *TParams
		NULL                               // MDNode *Decl
	);
#elif (LLVM_VERSION < 0x080000)
	llvm::DISubroutineType* llvmDiSubroutineType = (llvm::DISubroutineType*)function->getType()->getLlvmDiType();
	ASSERT(llvm::isa<llvm::DISubroutineType> (llvmDiSubroutineType));

	llvm::DISubprogram* subprogram = m_llvmDiBuilder->createFunction(
		unit->getLlvmDiFile(),             // DIScope *Scope
		function->getQualifiedName().sz(), // StringRef Name
		function->getQualifiedName().sz(), // StringRef LinkageName
		unit->getLlvmDiFile(),             // DIFile *File
		declPos.m_line + 1,                // unsigned LineNo
		llvmDiSubroutineType,              // DISubroutineType *Ty
		false,                             // bool isLocalToUnit
		true,                              // bool isDefinition
		scopePos.m_line + 1                // unsigned ScopeLine
	);

	function->getLlvmFunction()->setSubprogram(subprogram);
	return subprogram;
#else
	llvm::DISubroutineType* llvmDiSubroutineType = (llvm::DISubroutineType*)function->getType()->getLlvmDiType();
	ASSERT(llvm::isa<llvm::DISubroutineType> (llvmDiSubroutineType));

	llvm::DISubprogram* subprogram = m_llvmDiBuilder->createFunction(
		unit->getLlvmDiFile(),               // DIScope *Scope
		function->getQualifiedName().sz(),   // StringRef Name
		function->getQualifiedName().sz(),   // StringRef LinkageName
		unit->getLlvmDiFile(),               // DIFile *File
		declPos.m_line + 1,                  // unsigned LineNo
		llvmDiSubroutineType,                // DISubroutineType *Ty
		scopePos.m_line + 1,                 // unsigned ScopeLine
		llvm::DINode::FlagZero,              // DINode::DIFlags Flags
		llvm::DISubprogram::SPFlagDefinition // DISubroutine::DISPFlags SPFlags
	);

	function->getLlvmFunction()->setSubprogram(subprogram);
	return subprogram;
#endif
}

llvm::DILexicalBlock_vn
LlvmDiBuilder::createLexicalBlock(
	Scope* parentScope,
	const lex::LineCol& pos
) {
	Unit* unit = m_module->m_unitMgr.getCurrentUnit();
	ASSERT(unit);

	llvm::DIScope_vn llvmParentBlock;
	if (parentScope) {
		llvmParentBlock = parentScope->getLlvmDiScope();
	} else {
		Function* function = m_module->m_functionMgr.getCurrentFunction();
		ASSERT(function);

		llvmParentBlock = function->getLlvmDiSubprogram();
	}

	return m_llvmDiBuilder->createLexicalBlock(
		llvmParentBlock,
		unit->getLlvmDiFile(),
		pos.m_line + 1,
		pos.m_col + 1
#if (LLVM_VERSION == 0x030500)
		,0 // unsigned Discriminator
#endif
		);
}

//..............................................................................

} // namespace ct
} // namespace jnc
