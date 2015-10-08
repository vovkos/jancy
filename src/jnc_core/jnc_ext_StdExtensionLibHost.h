// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_CallSite.h"

namespace jnc {
namespace ext {

//.............................................................................

class StdExtensionLibHost: public ExtensionLibHost
{
protected:
	typedef sl::HashTableMap <
		sl::Guid, 
		size_t, 
		sl::HashDjb2 <sl::Guid>, 
		sl::CmpBin <sl::Guid> 
		> SlotMap;

protected:
	size_t m_nextLibSlot;
	SlotMap m_libSlotMap;

public:
	StdExtensionLibHost ()
	{
		m_nextLibSlot = 1;
	}

	virtual 
	size_t 
	getLibCacheSlot (const sl::Guid& libGuid);	

	virtual 
	Namespace*
	getModuleGlobalNamespace (Module* module)
	{
		return module->m_namespaceMgr.getGlobalNamespace ();
	}

	virtual 
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

	virtual 
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

	virtual
	Function*
	findNamespaceFunction (
		Namespace* nspace,
		const char* name
		)
	{
		return nspace->findFunctionByName (name);
	}

	virtual
	Property*
	findNamespaceProperty (
		Namespace* nspace,
		const char* name
		)
	{
		return nspace->findPropertyByName (name);
	}

	virtual
	Function*
	getFunctionOverload (
		Function* function,
		size_t overloadIdx
		)
	{
		return ext::getFunctionOverload (function, overloadIdx);
	}

	virtual
	Function*
	getPropertyGetter (Property* prop)
	{
		return ext::getPropertyGetter (prop);
	}

	virtual
	Function*
	getPropertySetter (Property* prop)
	{
		return ext::getPropertySetter (prop);
	}

	virtual
	Function*
	getTypePreConstructor (DerivableType* type)
	{
		return ext::getTypePreConstructor (type);
	}

	virtual
	Function*
	getTypeConstructor (DerivableType* type)
	{
		return ext::getTypeConstructor (type);
	}

	virtual
	Function*
	getClassTypeDestructor (ClassType* type)
	{
		return ext::getClassTypeDestructor (type);
	}

	virtual
	Function*
	getTypeUnaryOperator (
		DerivableType* type,
		ct::UnOpKind opKind	
		)
	{
		return ext::getTypeUnaryOperator (type, opKind);
	}

	virtual
	Function*
	getTypeBinaryOperator (
		DerivableType* type,
		ct::BinOpKind opKind	
		)
	{
		return ext::getTypeBinaryOperator (type, opKind);
	}

	virtual
	Function*
	getTypeCallOperator (DerivableType* type)
	{
		return ext::getTypeCallOperator (type);
	}

	virtual
	Function*
	getTypeCastOperator (
		DerivableType* type,
		size_t idx
		)
	{
		return ext::getTypeCastOperator (type, idx);
	}

	virtual 
	DerivableType*
	verifyModuleItemIsDerivableType (
		ModuleItem* item,
		const char* name
		)
	{
		return ct::verifyModuleItemIsDerivableType (item, name);
	}

	virtual 
	ClassType*
	verifyModuleItemIsClassType (
		ModuleItem* item,
		const char* name
		)
	{
		return ct::verifyModuleItemIsClassType (item, name);
	}

	virtual
	Namespace*
	getTypeNamespace (DerivableType* type)
	{
		return type;
	}

	virtual
	void
	mapFunction (
		Module* module,
		Function* function,
		void* p
		)
	{
		module->mapFunction (function, p);
	}

	virtual 
	void
	addSource (
		Module* module,
		const char* fileName,
		const char* source,
		size_t length
		)
	{
		module->m_importMgr.addSource (fileName, axl::sl::StringSlice (source, length));
	}

	virtual 
	Runtime*
	getCurrentThreadRuntime ()
	{
		return rt::getCurrentThreadRuntime ();
	}

	virtual
	void
	initializeRuntimeThread (
		Runtime* runtime,
		rt::ExceptionRecoverySnapshot* ers
		)
	{
		runtime->initializeThread (ers);
	}

	virtual
	void
	uninitializeRuntimeThread (
		Runtime* runtime,
		rt::ExceptionRecoverySnapshot* ers
		)
	{
		runtime->uninitializeThread (ers);
	}

	virtual
	void
	enterNoCollectRegion (Runtime* runtime)
	{
		runtime->m_gcHeap.enterNoCollectRegion ();
	}

	virtual
	void
	leaveNoCollectRegion (
		Runtime* runtime,
		bool canCollectNow
		)
	{
		runtime->m_gcHeap.leaveNoCollectRegion (canCollectNow);
	}

	virtual
	void
	enterWaitRegion (Runtime* runtime)
	{
		runtime->m_gcHeap.enterWaitRegion ();
	}

	virtual
	void
	leaveWaitRegion (Runtime* runtime)
	{
		runtime->m_gcHeap.leaveWaitRegion ();
	}

	virtual
	void*
	getFunctionMachineCode (Function* function)
	{
		return function->getMachineCode ();
	}

	virtual
	void*
	getMulticastCallMethodMachineCode (rt::Multicast* multicast)
	{
		ct::MulticastClassType* type = (ct::MulticastClassType*) multicast->m_box->m_type;
		return type->getMethod (ct::MulticastMethodKind_Call)->getMachineCode ();
	}

	virtual
	rt::IfaceHdr*
	allocateClass (
		Runtime* runtime,
		ClassType* type
		)
	{
		return runtime->m_gcHeap.allocateClass (type);
	}

	virtual
	rt::DataPtr
	allocateData (
		Runtime* runtime,
		Type* type
		)
	{
		return runtime->m_gcHeap.allocateData (type);
	}

	virtual
	rt::DataPtrValidator*
	createDataPtrValidator (
		rt::Runtime* runtime,
		rt::Box* box,
		void* rangeBegin,
		size_t rangeLength
		)
	{
		return runtime->m_gcHeap.createDataPtrValidator (box, rangeBegin, rangeLength);
	}

	virtual
	void
	primeClass (
		rt::Box* box,
		rt::Box* root,
		ClassType* type,
		void* vtable = NULL // if null then vtable of class type will be used
		)
	{
		return rt::primeClass (box, root, type, vtable);
	}

	virtual
	void
	gcWeakMark (
		rt::GcHeap* gcHeap,
		rt::Box* box
		)
	{
		gcHeap->weakMark (box);
	}

	virtual
	void
	gcMarkData (
		rt::GcHeap* gcHeap,
		rt::Box* box
		)
	{
		gcHeap->markData (box);
	}

	virtual
	void
	gcMarkClass (
		rt::GcHeap* gcHeap,
		rt::Box* box
		)
	{
		gcHeap->markClass (box);
	}

	virtual
	size_t 
	strLen (rt::DataPtr ptr)
	{
		return rt::strLen (ptr);
	}

	virtual
	rt::DataPtr
	strDup (
		const char* p,
		size_t length = -1
		)
	{
		return rt::strDup (p, length);
	}

	virtual
	rt::DataPtr
	memDup (
		const void* p,
		size_t size
		)
	{
		return rt::memDup (p, size);
	}

#if (_AXL_ENV == AXL_ENV_WIN)
	virtual
	int 
	handleGcSehException (
		Runtime* runtime,
		uint_t code, 
		EXCEPTION_POINTERS* exceptionPointers
		)
	{
		return runtime->m_gcHeap.handleSehException (code, exceptionPointers);
	}
#endif
};

//.............................................................................

} // namespace ext
} // namespace jnc
