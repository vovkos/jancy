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
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
OperatorMgr::getClassVTable (
	const Value& opValue,
	ClassType* classType,
	Value* resultValue
	)
{
	StructType* vtableType = classType->getVTableStructType ();
	ASSERT (vtableType);

	if (m_module->getCompileFlags () & ModuleCompileFlag_SimpleCheckNullPtr)
		checkNullPtr (opValue);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createBitCast (opValue, m_module->m_typeMgr.getStdType (StdType_IfaceHdrPtr), &ptrValue);
	m_module->m_llvmIrBuilder.createGep2 (ptrValue, 0, NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createLoad (ptrValue, NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createBitCast (ptrValue, vtableType->getDataPtrType_c (), resultValue);
	return true;
}

bool
OperatorMgr::getVirtualMethod (
	Function* function,
	Closure* closure,
	Value* resultValue
	)
{
	ASSERT (function->isVirtual ());

	if (!closure || !closure->isMemberClosure ())
	{
		err::setFormatStringError ("virtual function requires an object pointer");
		return false;
	}

	Value value = *closure->getArgValueList ()->getHead ();
	ClassType* classType = ((ClassPtrType*) value.getType ())->getTargetType ();
	ClassType* vtableType = function->getVirtualOriginClassType ();
	size_t VTableIndex = function->getClassVTableIndex ();

	BaseTypeCoord coord;
	classType->findBaseTypeTraverse (vtableType, &coord);
	VTableIndex += coord.m_vtableIndex;

	// class.vtbl*

	Value ptrValue;
	getClassVTable (value, classType, &ptrValue);

	// p*

	m_module->m_llvmIrBuilder.createGep2 (
		ptrValue,
		VTableIndex,
		NULL,
		&ptrValue
		);

	// p

	m_module->m_llvmIrBuilder.createLoad (
		ptrValue,
		NULL,
		&ptrValue
		);

	resultValue->setLlvmValue (
		ptrValue.getLlvmValue (),
		function->getType ()->getFunctionPtrType (FunctionPtrTypeKind_Thin)
		);

	resultValue->setClosure (closure);
	return true;
}

bool
OperatorMgr::getVirtualProperty (
	Property* prop,
	Closure* closure,
	Value* resultValue
	)
{
	ASSERT (prop->isVirtual ());

	if (!closure || !closure->isMemberClosure ())
	{
		err::setFormatStringError ("virtual property requires an object pointer");
		return false;
	}

	Value value = *closure->getArgValueList ()->getHead ();
	ClassType* classType = ((ClassPtrType*) value.getType ())->getTargetType ();
	size_t VTableIndex = prop->getParentClassVTableIndex ();

	BaseTypeCoord coord;
	classType->findBaseTypeTraverse (prop->getParentType (), &coord);
	VTableIndex += coord.m_vtableIndex;

	// class.vtbl*

	Value ptrValue;
	getClassVTable (value, classType, &ptrValue);

	// property.vtbl*

	m_module->m_llvmIrBuilder.createGep2 (
		ptrValue,
		VTableIndex,
		NULL,
		&ptrValue
		);

	m_module->m_llvmIrBuilder.createBitCast (
		ptrValue,
		prop->getType ()->getVTableStructType ()->getDataPtrType_c (),
		&ptrValue
		);

	resultValue->overrideType (ptrValue, prop->getType ()->getPropertyPtrType (PropertyPtrTypeKind_Thin));
	resultValue->setClosure (closure);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
