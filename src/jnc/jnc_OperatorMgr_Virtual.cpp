#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
OperatorMgr::getClassVTable (
	const Value& opValue,
	ClassType* classType,
	Value* resultValue
	)
{
	int32_t llvmIndexArray [] =
	{
		0, // class.iface*
		0, // class.iface.hdr*
		0, // class.vtbl**
	};

	Value ptrValue;
	m_module->m_llvmIrBuilder.createGep (
		opValue,
		llvmIndexArray,
		countof (llvmIndexArray),
		NULL,
		&ptrValue
		);

	// class.vtbl*

	DataPtrType* resultType = classType->getVTableStructType ()->getDataPtrType_c ();
	m_module->m_llvmIrBuilder.createLoad (ptrValue, resultType, resultValue);
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

	// pf*

	m_module->m_llvmIrBuilder.createGep2 (
		ptrValue,
		VTableIndex,
		NULL,
		&ptrValue
		);

	// pf

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

//.............................................................................

} // namespace jnc {
