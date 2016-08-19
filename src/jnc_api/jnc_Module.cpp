#include "pch.h"
#include "jnc_Module.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_GlobalNamespace*
jnc_Module_getGlobalNamespace (jnc_Module* module)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getGlobalNamespaceFunc (module);
}

JNC_EXTERN_C
jnc_Type*
jnc_Module_getPrimitiveType (
	jnc_Module* module,
	jnc_TypeKind typeKind
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getPrimitiveTypeFunc (module, typeKind);
}

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Module_findItem (
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_findItemFunc (module, name, libGuid, itemCacheSlot);
}

JNC_EXTERN_C
void
jnc_Module_mapFunction (
	jnc_Module* module,
	jnc_Function* function,
	void* p
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_mapFunctionFunc (module, function, p);
}

JNC_EXTERN_C
void
jnc_Module_addSource (
	jnc_Module* module,
	int isForced,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t size
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_addSourceFunc (module, isForced, lib, fileName, source, size);
}

JNC_EXTERN_C
void
jnc_Module_addImport (
	jnc_Module* module,
	const char* fileName
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_addImportFunc (module, fileName);
}

JNC_EXTERN_C
void
jnc_Module_addOpaqueClassTypeInfo (
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_addOpaqueClassTypeInfoFunc (module, qualifiedName, info);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Module* 
jnc_Module_create ()
{
	return AXL_MEM_NEW (jnc_Module);
}

JNC_EXTERN_C
void
jnc_Module_destroy (jnc_Module* module)
{
	AXL_MEM_DELETE (module);
}

JNC_EXTERN_C
void
jnc_Module_clear (jnc_Module* module)
{
	module->clear ();
}

JNC_EXTERN_C
void
jnc_Module_initialize (
	jnc_Module* module,
	const char* tag,
	uint_t compileFlags
	)
{
	module->initialize (tag, compileFlags);
}

JNC_EXTERN_C
uint_t
jnc_Module_getCompileFlags (jnc_Module* module)
{
	return module->getCompileFlags ();
}

JNC_EXTERN_C
jnc_ModuleCompileState
jnc_Module_getCompileState (jnc_Module* module)
{
	return module->getCompileState ();
}

JNC_EXTERN_C
jnc_GlobalNamespace*
jnc_Module_getGlobalNamespace (jnc_Module* module)
{
	return module->m_namespaceMgr.getGlobalNamespace ();
}

JNC_EXTERN_C
jnc_Type*
jnc_Module_getPrimitiveType (
	jnc_Module* module,
	jnc_TypeKind typeKind
	)
{
	return module->m_typeMgr.getPrimitiveType (typeKind);
}

JNC_EXTERN_C
jnc_Type*
jnc_Module_getStdType (
	jnc_Module* module,
	jnc_StdType stdType
	)
{
	return module->m_typeMgr.getStdType (stdType);
}

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Module_findItem (
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	)
{
	return module->m_extensionLibMgr.findItem (name, *libGuid, itemCacheSlot);
}

JNC_EXTERN_C
void
jnc_Module_mapFunction (
	jnc_Module* module,
	jnc_Function* function,
	void* p
	)
{
	module->mapFunction (function, p);
}

JNC_EXTERN_C
void
jnc_Module_addSource (
	jnc_Module* module,
	int isForced,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t length
	)
{
	axl::sl::StringRef contents (source, length);

	module->m_extensionLibMgr.addSource (lib, fileName, contents);

	if (isForced)
		module->m_importMgr.addImport (lib, fileName, contents);
}

JNC_EXTERN_C
void
jnc_Module_addImportDir (
	jnc_Module* module,
	const char* dir
	)
{
	module->m_importMgr.m_importDirList.insertTail (dir);
}

JNC_EXTERN_C
void
jnc_Module_addImport (
	jnc_Module* module,
	const char* fileName
	)
{
	module->m_importMgr.addImport (fileName);
}

JNC_EXTERN_C
void
jnc_Module_addOpaqueClassTypeInfo (
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
	)
{
	module->m_extensionLibMgr.addOpaqueClassTypeInfo (qualifiedName, info);
}

JNC_EXTERN_C
void
jnc_Module_addStaticLib (
	jnc_Module* module,
	jnc_ExtensionLib* lib
	)
{
	module->m_extensionLibMgr.addStaticLib (lib);
}

JNC_EXTERN_C
int
jnc_Module_parse (
	jnc_Module* module,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t length
	)
{
	return module->parse (lib, fileName, source, length);
}

JNC_EXTERN_C
int
jnc_Module_parseFile (
	jnc_Module* module,
	const char* fileName
	)
{
	return module->parseFile (fileName);
}

JNC_EXTERN_C
int
jnc_Module_parseImports (jnc_Module* module)
{
	return module->parseImports ();
}

JNC_EXTERN_C
int
jnc_Module_calcLayout (jnc_Module* module)
{
	return module->calcLayout ();
}

JNC_EXTERN_C
int
jnc_Module_compile (jnc_Module* module)
{
	return module->compile ();
}

JNC_EXTERN_C
int
jnc_Module_jit (jnc_Module* module)
{
	return module->jit ();
}

JNC_EXTERN_C
const char*
jnc_Module_createLlvmIrString_v (jnc_Module* module)
{
	return *jnc::getTlsStringBuffer () = module->createLlvmIrString ();
}

JNC_EXTERN_C
int
jnc_Module_generateDocumentation (
	jnc_Module* module,
	const char* outputDir
	)
{
	static char indexFileHdr [] = 
		"<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
		"<doxygenindex>\n";

	static char indexFileTerm [] = "</doxygenindex>\n";

	static char compoundFileHdr [] = 
		"<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
		"<doxygen>\n";

	static char compoundFileTerm [] = "</doxygen>\n";

	bool result;
		
	result = 
		module->calcLayout () && 
		io::ensureDirExists (outputDir);

	if (!result)
		return false;

	result = module->m_doxyMgr.resolveDoxyBlockTargets ();
	if (!result)
	{
		// generate a warning?
	}

	sl::String nspaceXml;
	sl::String indexXml;

	jnc::GlobalNamespace* nspace = module->m_namespaceMgr.getGlobalNamespace ();
	
	result = nspace->generateDocumentation (outputDir, &nspaceXml, &indexXml);
	if (!result)
		return false;

	if (nspaceXml.isEmpty ())
	{
		err::setStringError ("module does not contain any documentable items");
		return false;
	}

	sl::String refId = nspace->getDoxyBlock ()->getRefId ();
	sl::String nspaceFileName = sl::String (outputDir) + "/" + refId + ".xml";
	sl::String indexFileName = sl::String (outputDir) + "/index.xml";	

	io::File file;
	return
		file.open (nspaceFileName, io::FileFlag_Clear) &&
		file.write (compoundFileHdr, lengthof (compoundFileHdr)) != -1 &&
		file.write (nspaceXml, nspaceXml.getLength ()) != -1 &&		
		file.write (compoundFileTerm, lengthof (compoundFileTerm)) != -1 &&
		
		file.open (indexFileName, io::FileFlag_Clear) &&
		file.write (indexFileHdr, lengthof (indexFileHdr)) != -1 &&
		file.write (indexXml, indexXml.getLength ()) != -1 &&
		file.write (indexFileTerm, lengthof (indexFileTerm)) != -1;
}

JNC_EXTERN_C
void
jnc_initialize ()
{
#if 0 
	// orginally there was no llvm_shutdown in ioninja-server
	// so have to make sure it's not going to crash if we add it

	atexit (llvm::llvm_shutdown);
#endif

	llvm::InitializeNativeTarget ();
	llvm::InitializeNativeTargetAsmParser ();
	llvm::InitializeNativeTargetAsmPrinter ();
	llvm::InitializeNativeTargetDisassembler ();

	LLVMLinkInJIT();

	lex::registerParseErrorProvider ();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
