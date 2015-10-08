// This file is part of AXL (R) Library

// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#ifdef _JNC_SHARED_EXTENSION_LIB
#	include "jnc_ct_UnOpKind.h"
#	include "jnc_ct_BinOpKind.h"
#else
#	include "jnc_ct_Module.h"
#	include "jnc_rt_Runtime.h"
#endif

namespace jnc {
namespace ext {

#ifdef _JNC_SHARED_EXTENSION_LIB
	
// treat everything as opaque classes

class Runtime; 
class Module; 
class ModuleItem;
class Namespace;
class Function;
class Property;

// a tiny hack to reduce number of overloads

class Type
{
};

class DerivableType: public Type
{
};

class ClassType: public DerivableType
{
};

#else

typedef rt::Runtime       Runtime;
typedef ct::Module        Module;
typedef ct::ModuleItem    ModuleItem;
typedef ct::Namespace     Namespace;
typedef ct::Function      Function;
typedef ct::Property      Property;
typedef ct::Type          Type;
typedef ct::DerivableType DerivableType;
typedef ct::ClassType     ClassType;

#endif

//.............................................................................

class ExtensionLibHost
{
public:
	virtual 
	size_t 
	getLibCacheSlot (const sl::Guid& libGuid) = 0;

	virtual 
	Namespace*
	getModuleGlobalNamespace (Module* module) = 0;

	virtual 
	ModuleItem*
	findModuleItem (
		Module* module,
		const char* name,
		size_t libCacheSlot,
		size_t itemCacheSlot
		) = 0;

	virtual 
	ModuleItem*
	findModuleItem (
		Runtime* runtime,
		const char* name,
		size_t libCacheSlot,
		size_t itemCacheSlot
		) = 0;

	virtual
	Function*
	findNamespaceFunction (
		Namespace* nspace,
		const char* name
		) = 0;

	virtual
	Property*
	findNamespaceProperty (
		Namespace* nspace,
		const char* name
		) = 0;

	virtual
	Function*
	getFunctionOverload (
		Function* function,
		size_t overloadIdx
		) = 0;

	virtual
	Function*
	getPropertyGetter (Property* prop) = 0;

	virtual
	Function*
	getPropertySetter (Property* prop) = 0;

	virtual
	Function*
	getTypePreConstructor (DerivableType* type) = 0;

	virtual
	Function*
	getTypeConstructor (DerivableType* type) = 0;

	virtual
	Function*
	getClassTypeDestructor (ClassType* type) = 0;

	virtual
	Function*
	getTypeUnaryOperator (
		DerivableType* type,
		ct::UnOpKind opKind	
		) = 0;

	virtual
	Function*
	getTypeBinaryOperator (
		DerivableType* type,
		ct::BinOpKind opKind	
		) = 0;

	virtual
	Function*
	getTypeCallOperator (DerivableType* type) = 0;

	virtual
	Function*
	getTypeCastOperator (
		DerivableType* type,
		size_t idx
		) = 0;

	virtual 
	DerivableType*
	verifyModuleItemIsDerivableType (
		ModuleItem* item,
		const char* name
		) = 0;

	virtual 
	ClassType*
	verifyModuleItemIsClassType (
		ModuleItem* item,
		const char* name
		) = 0;

	virtual
	Namespace*
	getTypeNamespace (DerivableType* type) = 0;

	virtual
	void
	mapFunction (
		Module* module,
		Function* function,
		void* p
		) = 0;

	virtual 
	void
	addSource (
		Module* module,
		const char* fileName,
		const char* source,
		size_t size
		) = 0;

	virtual 
	Runtime*
	getCurrentThreadRuntime () = 0;

	virtual
	void
	initializeRuntimeThread (
		Runtime* runtime,
		rt::ExceptionRecoverySnapshot* ers
		) = 0;

	virtual
	void
	uninitializeRuntimeThread (
		Runtime* runtime,
		rt::ExceptionRecoverySnapshot* ers
		) = 0;

	virtual
	void
	enterNoCollectRegion (Runtime* runtime) = 0;

	virtual
	void
	leaveNoCollectRegion (
		Runtime* runtime,
		bool canCollectNow
		) = 0;

	virtual
	void
	enterWaitRegion (Runtime* runtime) = 0;

	virtual
	void
	leaveWaitRegion (Runtime* runtime) = 0;

	virtual
	void*
	getFunctionMachineCode (Function* function) = 0;

	virtual
	void*
	getMulticastCallMethodMachineCode (rt::Multicast* multicast) = 0;

	virtual
	rt::IfaceHdr*
	allocateClass (
		Runtime* runtime,
		ClassType* type
		) = 0;

	virtual
	rt::DataPtr
	allocateData (
		Runtime* runtime,
		Type* type
		) = 0;

	virtual
	rt::DataPtrValidator*
	createDataPtrValidator (
		Runtime* runtime,	
		rt::Box* box,
		void* rangeBegin,
		size_t rangeLength
		) = 0;

	virtual
	void
	primeClass (
		rt::Box* box,
		rt::Box* root,
		ClassType* type,
		void* vtable = NULL // if null then vtable of class type will be used
		) = 0;

	virtual
	void
	gcWeakMark (
		rt::GcHeap* gcHeap,
		rt::Box* box
		) = 0;

	virtual
	void
	gcMarkData (
		rt::GcHeap* gcHeap,
		rt::Box* box
		) = 0;

	virtual
	void
	gcMarkClass (
		rt::GcHeap* gcHeap,
		rt::Box* box
		) = 0;

	virtual
	size_t 
	strLen (rt::DataPtr ptr) = 0;

	virtual
	rt::DataPtr
	strDup (
		const char* p,
		size_t length = -1
		) = 0;

	virtual
	rt::DataPtr
	memDup (
		const void* p,
		size_t size
		) = 0;

#if (_AXL_ENV == AXL_ENV_WIN)
	virtual
	int 
	handleGcSehException (
		Runtime* runtime,
		uint_t code, 
		EXCEPTION_POINTERS* exceptionPointers
		) = 0;
#endif
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef _JNC_SHARED_EXTENSION_LIB

extern ExtensionLibHost* g_extensionLibHost;

inline
Namespace*
getModuleGlobalNamespace (Module* module)
{
	return g_extensionLibHost->getModuleGlobalNamespace (module);
}

inline 
ModuleItem*
findModuleItem (
	Module* module,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	)
{
	return g_extensionLibHost->findModuleItem (module, name, libCacheSlot, itemCacheSlot);
}

inline 
ModuleItem*
findModuleItem (
	Runtime* runtime,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	)
{
	return g_extensionLibHost->findModuleItem (runtime, name, libCacheSlot, itemCacheSlot);
}

inline 
Function*
findNamespaceFunction (
	Namespace* nspace,
	const char* name
	)
{
	return g_extensionLibHost->findNamespaceFunction (nspace, name);
}

inline 
Property*
findNamespaceProperty (
	Namespace* nspace,
	const char* name
	)
{
	return g_extensionLibHost->findNamespaceProperty (nspace, name);
}

inline
Function*
getFunctionOverload (
	Function* function,
	size_t overloadIdx
	)
{
	return g_extensionLibHost->getFunctionOverload (function, overloadIdx);
}

inline 
Function*
getPropertyGetter (Property* prop)
{
	return g_extensionLibHost->getPropertyGetter (prop);
}

inline 
Function*
getPropertySetter (Property* prop)
{
	return g_extensionLibHost->getPropertySetter (prop);
}

inline 
Function*
getTypePreConstructor (DerivableType* type)
{
	return g_extensionLibHost->getTypePreConstructor (type);
}

inline 
Function*
getTypeConstructor (DerivableType* type)
{
	return g_extensionLibHost->getTypeConstructor (type);
}

inline 
Function*
getClassTypeDestructor (ClassType* type)
{
	return g_extensionLibHost->getClassTypeDestructor (type);
}

inline 
Function*
getTypeUnaryOperator (
	DerivableType* type,
	ct::UnOpKind opKind	
	)
{
	return g_extensionLibHost->getTypeUnaryOperator (type, opKind);
}

inline 
Function*
getTypeBinaryOperator (
	DerivableType* type,
	ct::BinOpKind opKind	
	)
{
	return g_extensionLibHost->getTypeBinaryOperator (type, opKind);
}

inline 
Function*
getTypeCallOperator (DerivableType* type)
{
	return g_extensionLibHost->getTypeCallOperator (type);
}

inline 
Function*
getTypeCastOperator (
	DerivableType* type,
	size_t idx
	)
{
	return g_extensionLibHost->getTypeCastOperator (type, idx);
}

inline
DerivableType*
verifyModuleItemIsDerivableType (
	ModuleItem* item,
	const char* name
	)
{
	return g_extensionLibHost->verifyModuleItemIsDerivableType (item, name);
}

inline
ClassType*
verifyModuleItemIsClassType (
	ModuleItem* item,
	const char* name
	)
{
	return g_extensionLibHost->verifyModuleItemIsClassType (item, name);
}

inline
Namespace*
getTypeNamespace (DerivableType* type)
{
	return g_extensionLibHost->getTypeNamespace (type);
}

inline
void
mapFunction (
	Module* module,
	Function* function,
	void* p
	)
{
	g_extensionLibHost->mapFunction (module, function, p);
}

inline
void
addModuleSource (
	Module* module,
	const char* fileName,
	const char* source,
	size_t length
	)
{
	g_extensionLibHost->addSource (module, fileName, source, length);
}

#else

ExtensionLibHost*
getStdExtensionLibHost ();

inline
Namespace*
getModuleGlobalNamespace (Module* module)
{
	return module->m_namespaceMgr.getGlobalNamespace ();
}

inline 
ModuleItem*
findModuleItem (
	Module* module,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	)
{
	return module->m_extensionLibMgr.findItem (name, libCacheSlot, itemCacheSlot);
}

inline 
ModuleItem*
findModuleItem (
	Runtime* runtime,
	const char* name,
	size_t libCacheSlot,
	size_t itemCacheSlot
	)
{
	return runtime->getModule ()->m_extensionLibMgr.findItem (name, libCacheSlot, itemCacheSlot);
}

inline 
Function*
findNamespaceFunction (
	Namespace* nspace,
	const char* name
	)
{
	return nspace->findFunctionByName (name);
}

inline 
Property*
findNamespaceProperty (
	Namespace* nspace,
	const char* name
	)
{
	return nspace->findPropertyByName (name);
}

Function*
getFunctionOverload (
	Function* function,
	size_t overloadIdx
	);

inline
Function*
getPropertyGetter (Property* prop)
{
	return prop->getGetter ();
}

Function*
getPropertySetter (Property* prop);

Function*
getTypePreConstructor (DerivableType* type);

Function*
getTypeConstructor (DerivableType* type);

Function*
getClassTypeDestructor (ClassType* type);

Function*
getTypeUnaryOperator (
	DerivableType* type,
	ct::UnOpKind opKind	
	);

Function*
getTypeBinaryOperator (
	DerivableType* type,
	ct::BinOpKind opKind	
	);

Function*
getTypeCallOperator (DerivableType* type);

Function*
getTypeCastOperator (
	DerivableType* type,
	size_t idx
	);

inline
DerivableType*
verifyModuleItemIsDerivableType (
	ModuleItem* item,
	const char* name
	)
{
	return ct::verifyModuleItemIsDerivableType (item, name);
}

inline
ClassType*
verifyModuleItemIsClassType (
	ModuleItem* item,
	const char* name
	)
{
	return ct::verifyModuleItemIsClassType (item, name);
}

inline
Namespace*
getTypeNamespace (DerivableType* type)
{
	return type;
}

inline
void
mapFunction (
	Module* module,
	Function* function,
	void* p
	)
{
	module->mapFunction (function, p);
}

inline
void
addModuleSource (
	Module* module,
	const char* fileName,
	const char* source,
	size_t length
	)
{
	module->m_importMgr.addSource (fileName, axl::sl::StringSlice (source, length));
}

#endif

//.............................................................................

} // namespace ext
} // namespace jnc
