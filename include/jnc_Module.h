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

#pragma once

#define _JNC_MODULE_H

#include "jnc_AttributeBlock.h"
#include "jnc_Namespace.h"
#include "jnc_Alias.h"
#include "jnc_Variable.h"
#include "jnc_Function.h"
#include "jnc_Property.h"
#include "jnc_ArrayType.h"
#include "jnc_BitFieldType.h"
#include "jnc_EnumType.h"
#include "jnc_DerivableType.h"
#include "jnc_StructType.h"
#include "jnc_UnionType.h"
#include "jnc_ClassType.h"
#include "jnc_Unit.h"

/**

\defgroup module-subsystem Module Components

\defgroup module Module
	\ingroup module-subsystem
	\import{jnc_Module.h}

\addtogroup module
@{

\struct jnc_Module
	\ingroup module-subsystem
	\verbatim

	Opaque structure used as a handle to Jancy module.

	Use functions from the `Module` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_ModuleCompileFlag
{
	jnc_ModuleCompileFlag_DebugInfo                     = 0x00000001,
	jnc_ModuleCompileFlag_McJit                         = 0x00000002,
	jnc_ModuleCompileFlag_SimpleGcSafePoint             = 0x00000004,
	jnc_ModuleCompileFlag_GcSafePointInPrologue         = 0x00000010,
	jnc_ModuleCompileFlag_GcSafePointInInternalPrologue = 0x00000020,
	jnc_ModuleCompileFlag_Documentation                 = 0x00000200,
	jnc_ModuleCompileFlag_IgnoreOpaqueClassTypeInfo     = 0x00000400,
	jnc_ModuleCompileFlag_KeepTypedefShadow             = 0x00000800,
	jnc_ModuleCompileFlag_StdLibDoc                     = 0x00001000,
	jnc_ModuleCompileFlag_DisableDoxyComment1           = 0x00002000,
	jnc_ModuleCompileFlag_DisableDoxyComment2           = 0x00004000,
	jnc_ModuleCompileFlag_DisableDoxyComment3           = 0x00008000,
	jnc_ModuleCompileFlag_DisableDoxyComment4           = 0x00010000,

	jnc_ModuleCompileFlag_StdFlags =
#if (_JNC_OS_POSIX)
		jnc_ModuleCompileFlag_McJit |
#endif
		0
};

typedef enum jnc_ModuleCompileFlag jnc_ModuleCompileFlag;

//..............................................................................

enum jnc_ModuleCompileState
{
	jnc_ModuleCompileState_Idle,
	jnc_ModuleCompileState_Parsed,    // all files are parsed; global namespace is ready
	jnc_ModuleCompileState_Compiled,  // all required functions are compiled into LLVM IR
	jnc_ModuleCompileState_Jitted,    // machine code for all required functions is ready
};

typedef enum jnc_ModuleCompileState jnc_ModuleCompileState;

//..............................................................................

enum jnc_ModuleCompileErrorKind
{
	jnc_ModuleCompileErrorKind_ParseSyntax,   // -> llk::Parser::ErrorKind_Syntax
	jnc_ModuleCompileErrorKind_ParseSemantic, // -> llk::Parser::ErrorKind_Semantic
	jnc_ModuleCompileErrorKind_PostParse,     // post-parse stage
};

typedef enum jnc_ModuleCompileErrorKind jnc_ModuleCompileErrorKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// true  -> llk::Parser::RecoverAction_Synchronize
// false -> llk::Parser::RecoverAction_Fail

typedef
bool_t
jnc_ModuleCompileErrorHandlerFunc(
	void* context,
	jnc_ModuleCompileErrorKind errorKind
	);

//..............................................................................

JNC_EXTERN_C
jnc_Module*
jnc_Module_create();

JNC_EXTERN_C
void
jnc_Module_destroy(jnc_Module* module);

JNC_EXTERN_C
void
jnc_Module_clear(jnc_Module* module);

JNC_EXTERN_C
void
jnc_Module_initialize(
	jnc_Module* module,
	const char* tag,
	uint_t compileFlags
	);

JNC_EXTERN_C
uint_t
jnc_Module_getCompileFlags(jnc_Module* module);

JNC_EXTERN_C
jnc_ModuleCompileState
jnc_Module_getCompileState(jnc_Module* module);

JNC_EXTERN_C
size_t
jnc_Module_getCompileErrorCount(jnc_Module* module);

JNC_EXTERN_C
void
jnc_Module_setCompileErrorHandler(
	jnc_Module* module,
	jnc_ModuleCompileErrorHandlerFunc* handler,
	void* context
	);

JNC_EXTERN_C
jnc_GlobalNamespace*
jnc_Module_getGlobalNamespace(jnc_Module* module);

JNC_EXTERN_C
jnc_Type*
jnc_Module_getPrimitiveType(
	jnc_Module* module,
	jnc_TypeKind typeKind
	);

JNC_EXTERN_C
jnc_Type*
jnc_Module_getStdType(
	jnc_Module* module,
	jnc_StdType stdType
	);

JNC_EXTERN_C
jnc_FindModuleItemResult
jnc_Module_findExtensionLibItem(
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	);

JNC_EXTERN_C
bool_t
jnc_Module_mapVariable(
	jnc_Module* module,
	jnc_Variable* variable,
	void* p
	);

JNC_EXTERN_C
bool_t
jnc_Module_mapFunction(
	jnc_Module* module,
	jnc_Function* function,
	void* p
	);

JNC_EXTERN_C
void
jnc_Module_addSource(
	jnc_Module* module,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t length
	);

JNC_EXTERN_C
void
jnc_Module_addImportDir(
	jnc_Module* module,
	const char* dir
	);

JNC_EXTERN_C
bool_t
jnc_Module_addImport(
	jnc_Module* module,
	const char* fileName
	);

JNC_EXTERN_C
void
jnc_Module_addIgnoredImport(
	jnc_Module* module,
	const char* fileName
	);

JNC_EXTERN_C
void
jnc_Module_addOpaqueClassTypeInfo(
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
	);

JNC_EXTERN_C
void
jnc_Module_addStaticLib(
	jnc_Module* module,
	jnc_ExtensionLib* lib
	);

JNC_EXTERN_C
void
jnc_Module_require(
	jnc_Module* module,
	jnc_ModuleItemKind itemKind,
	const char* name,
	bool_t isEssential
	);

JNC_EXTERN_C
void
jnc_Module_requireType(
	jnc_Module* module,
	jnc_TypeKind typeKind,
	const char* name,
	bool_t isEssential
	);

JNC_EXTERN_C
bool_t
jnc_Module_parse(
	jnc_Module* module,
	const char* fileName,
	const char* source,
	size_t length
	);

JNC_EXTERN_C
bool_t
jnc_Module_parseFile(
	jnc_Module* module,
	const char* fileName
	);

JNC_EXTERN_C
bool_t
jnc_Module_parseImports(jnc_Module* module);

JNC_EXTERN_C
bool_t
jnc_Module_compile(jnc_Module* module);

JNC_EXTERN_C
bool_t
jnc_Module_optimize(
	jnc_Module* module,
	uint_t level
	);

JNC_EXTERN_C
bool_t
jnc_Module_jit(jnc_Module* module);

JNC_EXTERN_C
bool_t
jnc_Module_generateDocumentation(
	jnc_Module* module,
	const char* outputDir
	);

JNC_EXTERN_C
const char*
jnc_Module_getLlvmIrString_v(jnc_Module* module);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_Module
{
	static
	jnc_Module*
	create()
	{
		return jnc_Module_create();
	}

	void
	destroy()
	{
		jnc_Module_destroy(this);
	}

	void
	clear()
	{
		jnc_Module_clear(this);
	}

	void
	initialize(
		const char* tag,
		uint_t compileFlags = jnc_ModuleCompileFlag_StdFlags
		)
	{
		jnc_Module_initialize(this, tag, compileFlags);
	}

	uint_t
	getCompileFlags()
	{
		return jnc_Module_getCompileFlags(this);
	}

	jnc_ModuleCompileState
	getCompileState()
	{
		return jnc_Module_getCompileState(this);
	}

	size_t
	getCompileErrorCount()
	{
		return jnc_Module_getCompileErrorCount(this);
	}

	void
	setCompileErrorHandler(
		jnc_ModuleCompileErrorHandlerFunc* errorHandler,
		void* context
		)
	{
		jnc_Module_setCompileErrorHandler(this, errorHandler, context);
	}

	jnc_GlobalNamespace*
	getGlobalNamespace()
	{
		return jnc_Module_getGlobalNamespace(this);
	}

	jnc_Type*
	getPrimitiveType(jnc_TypeKind typeKind)
	{
		return jnc_Module_getPrimitiveType(this, typeKind);
	}

	jnc_Type*
	getStdType(jnc_StdType stdType)
	{
		return jnc_Module_getStdType(this, stdType);
	}

	jnc_FindModuleItemResult
	findExtenstionLibItem(
		const char* name,
		const jnc_Guid* libGuid,
		size_t itemCacheSlot
		)
	{
		return jnc_Module_findExtensionLibItem(this, name, libGuid, itemCacheSlot);
	}

	bool
	mapVariable(
		jnc_Variable* variable,
		void* p
		)
	{
		return jnc_Module_mapVariable(this, variable, p) != 0;
	}

	bool
	mapFunction(
		jnc_Function* function,
		void* p
		)
	{
		return jnc_Module_mapFunction(this, function, p) != 0;
	}

	void
	addSource(
		jnc_ExtensionLib* lib,
		const char* fileName,
		const char* source,
		size_t length = -1
		)
	{
		jnc_Module_addSource(this, lib, fileName, source, length);
	}

	void
	addImportDir(const char* dir)
	{
		jnc_Module_addImportDir(this, dir);
	}

	bool
	addImport(const char* fileName)
	{
		return jnc_Module_addImport(this, fileName) != 0;
	}

	void
	addIgnoredImport(const char* fileName)
	{
		jnc_Module_addIgnoredImport(this, fileName);
	}

	void
	addOpaqueClassTypeInfo(
		const char* qualifiedName,
		const jnc_OpaqueClassTypeInfo* info
		)
	{
		jnc_Module_addOpaqueClassTypeInfo(this, qualifiedName, info);
	}

	void
	addStaticLib(jnc_ExtensionLib* lib)
	{
		jnc_Module_addStaticLib(this, lib);
	}

	void
	require(
		jnc_ModuleItemKind itemKind,
		const char* name,
		bool isEssential = true
		)
	{
		jnc_Module_require(this, itemKind, name, isEssential);
	}

	void
	require(
		jnc_TypeKind typeKind,
		const char* name,
		bool isEssential = true
		)
	{
		jnc_Module_requireType(this, typeKind, name, isEssential);
	}

	bool
	parse(
		const char* fileName,
		const char* source,
		size_t length = -1
		)
	{
		return jnc_Module_parse(this, fileName, source, length) != 0;
	}

	bool
	parseFile(const char* fileName)
	{
		return jnc_Module_parseFile(this, fileName) != 0;
	}

	bool
	parseImports()
	{
		return jnc_Module_parseImports(this) != 0;
	}

	bool
	compile()
	{
		return jnc_Module_compile(this) != 0;
	}

	bool
	optimize(uint_t level = 2)
	{
		return jnc_Module_optimize(this, level) != 0;
	}

	bool
	jit()
	{
		return jnc_Module_jit(this) != 0;
	}

	const char*
	getLlvmIrString_v()
	{
		return jnc_Module_getLlvmIrString_v(this);
	}

	bool
	generateDocumentation(const char* outputDir)
	{
		return jnc_Module_generateDocumentation(this, outputDir) != 0;
	}
};
#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_ModuleCompileState ModuleCompileState;

const ModuleCompileState
	ModuleCompileState_Idle     = jnc_ModuleCompileState_Idle,
	ModuleCompileState_Parsed   = jnc_ModuleCompileState_Parsed,
	ModuleCompileState_Compiled = jnc_ModuleCompileState_Compiled,
	ModuleCompileState_Jitted   = jnc_ModuleCompileState_Jitted;

//..............................................................................

typedef jnc_ModuleCompileFlag ModuleCompileFlag;

const ModuleCompileFlag
	ModuleCompileFlag_DebugInfo                     = jnc_ModuleCompileFlag_DebugInfo,
	ModuleCompileFlag_McJit                         = jnc_ModuleCompileFlag_McJit,
	ModuleCompileFlag_SimpleGcSafePoint             = jnc_ModuleCompileFlag_SimpleGcSafePoint,
	ModuleCompileFlag_GcSafePointInPrologue         = jnc_ModuleCompileFlag_GcSafePointInPrologue,
	ModuleCompileFlag_GcSafePointInInternalPrologue = jnc_ModuleCompileFlag_GcSafePointInInternalPrologue,
	ModuleCompileFlag_Documentation                 = jnc_ModuleCompileFlag_Documentation,
	ModuleCompileFlag_IgnoreOpaqueClassTypeInfo     = jnc_ModuleCompileFlag_IgnoreOpaqueClassTypeInfo,
	ModuleCompileFlag_KeepTypedefShadow             = jnc_ModuleCompileFlag_KeepTypedefShadow,
	ModuleCompileFlag_StdLibDoc                     = jnc_ModuleCompileFlag_StdLibDoc,
	ModuleCompileFlag_DisableDoxyComment1           = jnc_ModuleCompileFlag_DisableDoxyComment1,
	ModuleCompileFlag_DisableDoxyComment2           = jnc_ModuleCompileFlag_DisableDoxyComment2,
	ModuleCompileFlag_DisableDoxyComment3           = jnc_ModuleCompileFlag_DisableDoxyComment3,
	ModuleCompileFlag_DisableDoxyComment4           = jnc_ModuleCompileFlag_DisableDoxyComment4,
	ModuleCompileFlag_StdFlags                      = jnc_ModuleCompileFlag_StdFlags;

//..............................................................................

typedef jnc_ModuleCompileErrorHandlerFunc ModuleCompileErrorHandlerFunc;
typedef jnc_ModuleCompileErrorKind ModuleCompileErrorKind;

const ModuleCompileErrorKind
	ModuleCompileErrorKind_ParseSyntax   = jnc_ModuleCompileErrorKind_ParseSyntax,
	ModuleCompileErrorKind_ParseSemantic = jnc_ModuleCompileErrorKind_ParseSemantic,
	ModuleCompileErrorKind_PostParse     = jnc_ModuleCompileErrorKind_PostParse;

//..............................................................................

class AutoModule
{
protected:
	Module* m_module;

public:
	AutoModule()
	{
		m_module = jnc_Module_create();
	}

	~AutoModule()
	{
		if (m_module)
			jnc_Module_destroy(m_module);
	}

	operator Module* () const
	{
		return m_module;
	}

	Module*
	operator -> () const
	{
		return m_module;
	}

	Module*
	p() const
	{
		return m_module;
	}
};

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
