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
#include "jnc_Module.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_ct_Jit.h"
#	include "jnc_rt_ExceptionMgr.h"
#endif

#define _JNC_LLVM_DEBUG 0

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
const char*
jnc_Module_getName(jnc_Module* module) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getNameFunc(module);
}

JNC_EXTERN_C
jnc_GlobalNamespace*
jnc_Module_getGlobalNamespace(jnc_Module* module) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getGlobalNamespaceFunc(module);
}

JNC_EXTERN_C
jnc_GlobalNamespace*
jnc_Module_getStdNamespace(
	jnc_Module* module,
	jnc_StdNamespace stdNamespace
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getStdNamespaceFunc(module, stdNamespace);
}

JNC_EXTERN_C
jnc_Type*
jnc_Module_getPrimitiveType(
	jnc_Module* module,
	jnc_TypeKind typeKind
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getPrimitiveTypeFunc(module, typeKind);
}

JNC_EXTERN_C
jnc_Type*
jnc_Module_getStdType(
	jnc_Module* module,
	jnc_StdType stdType
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getStdTypeFunc(module, stdType);
}

JNC_EXTERN_C
handle_t
jnc_Module_getExtensionSourceFileIterator(jnc_Module* module) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getExtensionSourceFileIteratorFunc(module);
}

JNC_EXTERN_C
const char*
jnc_Module_getNextExtensionSourceFile(
	jnc_Module* module,
	handle_t* iterator
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getNextExtensionSourceFileFunc(module, iterator);
}

JNC_EXTERN_C
jnc_FindModuleItemResult
jnc_Module_findExtensionLibItem(
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_findExtensionLibItemFunc(module, name, libGuid, itemCacheSlot);
}

JNC_EXTERN_C
const char*
jnc_Module_getExtensionLibFilePath(
	jnc_Module* module,
	jnc_ExtensionLib* lib
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getExtensionLibFilePathFunc(module, lib);
}

JNC_EXTERN_C
bool_t
jnc_Module_mapVariable(
	jnc_Module* module,
	jnc_Variable* variable,
	void* p
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_mapVariableFunc(module, variable, p);
}

JNC_EXTERN_C
bool_t
jnc_Module_mapFunction(
	jnc_Module* module,
	jnc_Function* function,
	void* p
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_mapFunctionFunc(module, function, p);
}

JNC_EXTERN_C
void
jnc_Module_addSource(
	jnc_Module* module,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t size
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_addSourceFunc(module, lib, fileName, source, size);
}

JNC_EXTERN_C
handle_t
jnc_Module_getImportDirIterator(jnc_Module* module) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getImportDirIteratorFunc(module);
}

JNC_EXTERN_C
const char*
jnc_Module_getNextImportDir(
	jnc_Module* module,
	handle_t* iterator
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getNextImportDirFunc(module, iterator);
}

JNC_EXTERN_C
bool_t
jnc_Module_addImport(
	jnc_Module* module,
	const char* fileName
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_addImportFunc(module, fileName);
}

JNC_EXTERN_C
void
jnc_Module_addSourceImport(
	jnc_Module* module,
	const char* fileName,
	const char* source,
	size_t length
) {
	jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_addSourceImportFunc(module, fileName, source, length);
}

JNC_EXTERN_C
void
jnc_Module_addOpaqueClassTypeInfo(
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
) {
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_addOpaqueClassTypeInfoFunc(module, qualifiedName, info);
}

JNC_EXTERN_C
void
jnc_Module_require(
	jnc_Module* module,
	jnc_ModuleItemKind itemKind,
	const char* name,
	bool_t isEssential
) {
	jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_requireFunc(module, itemKind, name, isEssential);
}

JNC_EXTERN_C
void
jnc_Module_requireType(
	jnc_Module* module,
	jnc_TypeKind typeKind,
	const char* name,
	bool_t isEssential
) {
	jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_requireTypeFunc(module, typeKind, name, isEssential);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Module*
jnc_Module_create() {
	return new jnc_Module;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_destroy(jnc_Module* module) {
	delete module;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_clear(jnc_Module* module) {
	module->clear();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_initialize(
	jnc_Module* module,
	const char* name,
	const jnc_ModuleConfig* config
) {
	module->initialize(name, config);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_unloadDynamicLibs(jnc_Module* module) {
	module->m_extensionLibMgr.unloadDynamicLibs();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_updateCapabilities(jnc_Module* module) {
	module->m_extensionLibMgr.updateCapabilities();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_setDynamicExtensionAuthenticatorConfig(
	jnc_Module* module,
	const jnc_CodeAuthenticatorConfig* config
) {
	module->m_extensionLibMgr.setDynamicExtensionAuthenticatorConfig(config);
}

JNC_EXTERN_C
const char*
jnc_Module_getName(jnc_Module* module) {
	return module->getName().sz();
}

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_Module_getCompileFlags(jnc_Module* module) {
	return module->getCompileFlags();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_ModuleCompileState
jnc_Module_getCompileState(jnc_Module* module) {
	return module->getCompileState();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Module_getCompileErrorCountLimit(jnc_Module* module) {
	return module->m_compileErrorCountLimit;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_setCompileErrorCountLimit(
	jnc_Module* module,
	size_t limit
) {
	module->m_compileErrorCountLimit = limit;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Module_getCompileErrorCount(jnc_Module* module) {
	return module->getCompileErrorCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_setCompileErrorHandler(
	jnc_Module* module,
	jnc_ModuleCompileErrorHandlerFunc* handler,
	void* context
) {
	module->setCompileErrorHandler(handler, context);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_setDynamicSectionObserver(
	jnc_Module* module,
	jnc_ModuleDynamicSectionObserverFunc* observer,
	void* context
) {
	module->setDynamicSectionObserver(observer, context);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_GlobalNamespace*
jnc_Module_getGlobalNamespace(jnc_Module* module) {
	return module->m_namespaceMgr.getGlobalNamespace();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_GlobalNamespace*
jnc_Module_getStdNamespace(
	jnc_Module* module,
	jnc_StdNamespace stdNamespace
) {
	return module->m_namespaceMgr.getStdNamespace(stdNamespace);
}


JNC_EXTERN_C
JNC_EXPORT_O
jnc_Type*
jnc_Module_getPrimitiveType(
	jnc_Module* module,
	jnc_TypeKind typeKind
) {
	return module->m_typeMgr.getPrimitiveType(typeKind);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Type*
jnc_Module_getStdType(
	jnc_Module* module,
	jnc_StdType stdType
) {
	return module->m_typeMgr.getStdType(stdType);
}

JNC_EXTERN_C
JNC_EXPORT_O
handle_t
jnc_Module_getExtensionSourceFileIterator(jnc_Module* module) {
	return (handle_t)module->m_extensionLibMgr.getSourceFileMap().getHead().p();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Module_getNextExtensionSourceFile(
	jnc_Module* module,
	handle_t* iterator
) {
	sl::StringHashTableIterator<void*> it((sl::HashTableEntry<sl::String, void*>*)*iterator);
	if (!it)
		return NULL;

	const char* fileName  = it->getKey().sz();
	*iterator = (++it).p();
	return fileName;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FindModuleItemResult
jnc_Module_findExtensionLibItem(
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
) {
	return module->m_extensionLibMgr.findItem(name, *libGuid, itemCacheSlot);
}

JNC_EXTERN_C
const char*
jnc_Module_getExtensionLibFilePath(
	jnc_Module* module,
	jnc_ExtensionLib* lib
) {
	return module->m_extensionLibMgr.getExtensionLibFilePath(lib).sz();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_mapVariable(
	jnc_Module* module,
	jnc_Variable* variable,
	void* p
) {
	return module->m_jit->mapVariable(variable, p);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_mapFunction(
	jnc_Module* module,
	jnc_Function* function,
	void* p
) {
	return module->m_jit->mapFunction(function, p);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_addSource(
	jnc_Module* module,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t length
) {
	module->m_extensionLibMgr.addSource(
		lib,
		fileName,
		length != -1 ?
			sl::StringRef(source, length) :
			sl::StringRef(source)
	);
}

JNC_EXTERN_C
JNC_EXPORT_O
handle_t
jnc_Module_getImportDirIterator(jnc_Module* module) {
	return (handle_t)module->m_importMgr.m_importDirList.getHead().getEntry();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Module_getNextImportDir(
	jnc_Module* module,
	handle_t* iterator
) {
	sl::BoxIterator<sl::String> it((sl::BoxListEntry<sl::String>*)*iterator);
	if (!it)
		return NULL;

	const char* dir = *it;
	*iterator = (++it).getEntry();
	return dir;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_addImportDir(
	jnc_Module* module,
	const char* dir
) {
	module->m_importMgr.m_importDirList.insertTail(dir);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_addImport(
	jnc_Module* module,
	const char* fileName
) {
	return module->m_importMgr.addImport(fileName);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_addSourceImport(
	jnc_Module* module,
	const char* fileName,
	const char* source,
	size_t length
) {
	module->m_importMgr.addImport(
		NULL,
		fileName,
		length != -1 ?
			sl::StringRef(source, length) :
			sl::StringRef(source)
	);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_addIgnoredImport(
	jnc_Module* module,
	const char* fileName
) {
	module->m_importMgr.addIgnoredImport(fileName);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_addOpaqueClassTypeInfo(
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
) {
	module->m_extensionLibMgr.addOpaqueClassTypeInfo(qualifiedName, info);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_addStaticLib(
	jnc_Module* module,
	jnc_ExtensionLib* lib
) {
	module->m_extensionLibMgr.addStaticLib(lib);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_require(
	jnc_Module* module,
	jnc_ModuleItemKind itemKind,
	const char* name,
	bool_t isEssential
) {
	module->require(itemKind, name, isEssential != 0);
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_requireType(
	jnc_Module* module,
	jnc_TypeKind typeKind,
	const char* name,
	bool_t isEssential
) {
	module->require(typeKind, name, isEssential != 0);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_parse(
	jnc_Module* module,
	const char* fileName,
	const char* source,
	size_t length
) {
	return module->parse(
		fileName,
		length != -1 ?
			sl::StringRef(source, length) :
			sl::StringRef(source)
		);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_parseFile(
	jnc_Module* module,
	const char* fileName
) {
	return module->parseFile(fileName);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_parseImports(jnc_Module* module) {
	return module->parseImports();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_compile(jnc_Module* module) {
	return module->compile();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_optimize(
	jnc_Module* module,
	uint_t level
) {
	return module->optimize(level);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_jit(jnc_Module* module) {
	return module->jit();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Module_getLlvmIrString_v(jnc_Module* module) {
	return *jnc::getTlsStringBuffer() = module->getLlvmIrString();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Module_generateDocumentation(
	jnc_Module* module,
	const char* outputDir
) {
	return
		module->m_namespaceMgr.getGlobalNamespace()->ensureNamespaceReadyDeep() &&
		module->m_doxyModule.generateDocumentation(
			outputDir,
			"index.xml",
			JNC_GLOBAL_NAMESPACE_DOXID ".xml"
		);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_CodeAssist*
jnc_Module_generateCodeAssist(
	jnc_Module* module,
	jnc_CodeAssistKind kind,
	jnc_Module* cacheModule,
	size_t offset,
	const char* source,
	size_t length
) {
	return module->generateCodeAssist(kind, cacheModule, offset, sl::StringRef(source, length));
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_Module_cancelCodeAssist(jnc_Module* module) {
	module->cancelCodeAssist();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_CodeAssist*
jnc_Module_getCodeAssist(jnc_Module* module) {
	return module->m_codeAssistMgr.getCodeAssist();
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_initialize(const char* reserved) {
	atexit(llvm::llvm_shutdown);

	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmParser();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetDisassembler();

#if (_JNC_LLVM_DEBUG)
	llvm::DebugFlag = true;
#endif

#if (_JNC_LLVM_JIT_LEGACY)
	LLVMLinkInJIT();
#endif
	LLVMLinkInMCJIT();

	sl::getSimpleSingleton<jnc::rt::ExceptionMgr>()->install();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
