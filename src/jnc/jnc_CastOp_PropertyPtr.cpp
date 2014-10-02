#include "pch.h"
#include "jnc_CastOp_PropertyPtr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CastKind
Cast_PropertyPtr_FromDataPtr::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_DataPtr && type->getTypeKind () == TypeKind_PropertyPtr);

	DataPtrType* srcPtrType = (DataPtrType*) opValue.getType ();
	PropertyPtrType* dstPtrType = (PropertyPtrType*) type;
	PropertyType* propertyType = dstPtrType->getTargetType ();

	return !propertyType->isIndexed ()  ?
		m_module->m_operatorMgr.getCastKind (srcPtrType->getTargetType (), propertyType->getReturnType ()) :
		CastKind_None;
}

bool
Cast_PropertyPtr_FromDataPtr::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_DataPtr && type->getTypeKind () == TypeKind_PropertyPtr);

	PropertyPtrType* dstPtrType = (PropertyPtrType*) type;

	if (opValue.getValueKind () == ValueKind_Variable &&
		opValue.getVariable ()->getStorageKind () == StorageKind_Static &&
		opValue.getLlvmValue () == opValue.getVariable ()->getLlvmValue ())
	{
		return llvmCast_DirectThunk (opValue.getVariable (), dstPtrType, resultValue);
	}

	if (dstPtrType->getPtrTypeKind () == PropertyPtrTypeKind_Thin)
	{
		setCastError (opValue, type);
		return false;
	}

	return llvmCast_FullClosure (storageKind, opValue, dstPtrType, resultValue);
}

bool
Cast_PropertyPtr_FromDataPtr::llvmCast_DirectThunk (
	Variable* variable,
	PropertyPtrType* dstPtrType,
	Value* resultValue
	)
{
	Property* thunkProperty = m_module->m_functionMgr.getDirectDataThunkProperty (
		variable,
		dstPtrType->getTargetType (),
		dstPtrType->hasClosure ()
		);

	Value propertyValue = thunkProperty;
	m_module->m_operatorMgr.unaryOperator (UnOpKind_Addr, &propertyValue);

	Value closureValue;

	if (dstPtrType->hasClosure ())
	{
		closureValue = m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr)->getZeroValue ();
		propertyValue.insertToClosureHead (closureValue);
	}

	return m_module->m_operatorMgr.castOperator (propertyValue, dstPtrType, resultValue);
}

bool
Cast_PropertyPtr_FromDataPtr::llvmCast_FullClosure (
	StorageKind storageKind,
	const Value& opValue,
	PropertyPtrType* dstPtrType,
	Value* resultValue
	)
{
	Value closureValue;
	bool result = m_module->m_operatorMgr.createDataClosureObject (
		storageKind,
		opValue,
		dstPtrType->getTargetType (),
		&closureValue
		);

	if (!result)
		return false;

	ASSERT (isClassPtrType (closureValue.getType (), ClassTypeKind_PropertyClosure));

	PropertyClosureClassType* closureType = (PropertyClosureClassType*) ((ClassPtrType*) closureValue.getType ())->getTargetType ();
	m_module->m_llvmIrBuilder.createClosurePropertyPtr (closureType->getThunkProperty (), closureValue, dstPtrType, resultValue);
	return true;
}

//.............................................................................

CastKind
Cast_PropertyPtr_Base::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_PropertyPtr && type->getTypeKind () == TypeKind_PropertyPtr);

	PropertyPtrType* srcPtrType = (PropertyPtrType*) opValue.getClosureAwareType ();
	PropertyPtrType* dstPtrType = (PropertyPtrType*) type;

	if (!srcPtrType)
		return CastKind_None;

	if (srcPtrType->isConstPtrType () && !dstPtrType->isConstPtrType ())
		return CastKind_None;

	return m_module->m_operatorMgr.getPropertyCastKind (
		srcPtrType->getTargetType (),
		dstPtrType->getTargetType ()
		);
}

//.............................................................................

bool
Cast_PropertyPtr_FromFat::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_PropertyPtr && type->getTypeKind () == TypeKind_PropertyPtr);

	PropertyPtrType* srcPtrType = (PropertyPtrType*) opValue.getType ();
	PropertyType* srcPropertyType = srcPtrType->getTargetType ();

	PropertyPtrType* thinPtrType = srcPropertyType->getStdObjectMemberPropertyType ()->getPropertyPtrType (PropertyPtrTypeKind_Thin);

	Value pfnValue;
	Value closureObjValue;
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 0, thinPtrType, &pfnValue);
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 1, m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr), &closureObjValue);

	pfnValue.setClosure (opValue.getClosure ());
	pfnValue.insertToClosureHead (closureObjValue);

	return m_module->m_operatorMgr.castOperator (storageKind, pfnValue, type, resultValue);
}

//.............................................................................

bool
Cast_PropertyPtr_Thin2Fat::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_PropertyPtr && type->getTypeKind () == TypeKind_PropertyPtr);

	PropertyPtrType* srcPtrType = (PropertyPtrType*) opValue.getType ();
	PropertyPtrType* dstPtrType = (PropertyPtrType*) type;

	PropertyType* srcPropertyType = srcPtrType->getTargetType ();
	PropertyType* dstPropertyType = dstPtrType->getTargetType ();

	Closure* closure = opValue.getClosure ();

	Value simpleClosureObjValue;

	bool isSimpleClosure = closure && closure->isSimpleClosure ();
	if (isSimpleClosure)
		simpleClosureObjValue = *closure->getArgValueList ()->getHead ();

	// case 1: no conversion required, no closure object needs to be created

	if (isSimpleClosure &&
		srcPropertyType->isMemberPropertyType () &&
		srcPropertyType->getShortType ()->cmp (dstPropertyType) == 0)
	{
		return llvmCast_NoThunkSimpleClosure (
			opValue,
			simpleClosureObjValue,
			srcPropertyType,
			dstPtrType,
			resultValue
			);
	}

	if (opValue.getValueKind () == ValueKind_Property)
	{
		Property* prop = opValue.getProperty ();

		// case 2.1: conversion is required, but no closure object needs to be created (closure arg is null)

		if (!closure)
			return llvmCast_DirectThunkNoClosure (
				prop,
				dstPtrType,
				resultValue
				);

		// case 2.2: same as above, but simple closure is passed as closure arg

		if (isSimpleClosure && prop->getType ()->isMemberPropertyType ())
			return llvmCast_DirectThunkSimpleClosure (
				prop,
				simpleClosureObjValue,
				dstPtrType,
				resultValue
				);
	}

	// case 3: closure object needs to be created (so conversion is required even if Property signatures match)

	return llvmCast_FullClosure (
		storageKind,
		opValue,
		srcPropertyType,
		dstPtrType,
		resultValue
		);
}

bool
Cast_PropertyPtr_Thin2Fat::llvmCast_NoThunkSimpleClosure (
	const Value& opValue,
	const Value& simpleClosureObjValue,
	PropertyType* srcPropertyType,
	PropertyPtrType* dstPtrType,
	Value* resultValue
	)
{
	Type* thisArgType = srcPropertyType->getThisArgType ();

	Value thisArgValue;
	bool result = m_module->m_operatorMgr.castOperator (simpleClosureObjValue, thisArgType, &thisArgValue);
	if (!result)
		return false;

	if (opValue.getValueKind () == ValueKind_Property)
		return createClosurePropertyPtr (opValue.getProperty (), thisArgValue, dstPtrType, resultValue);

	m_module->m_llvmIrBuilder.createClosurePropertyPtr (opValue, thisArgValue, dstPtrType, resultValue);
	return true;
}

bool
Cast_PropertyPtr_Thin2Fat::llvmCast_DirectThunkNoClosure (
	Property* prop,
	PropertyPtrType* dstPtrType,
	Value* resultValue
	)
{
	Property* thunkProperty = m_module->m_functionMgr.getDirectThunkProperty (
		prop,
		((PropertyPtrType*) dstPtrType)->getTargetType (),
		true
		);

	Value nullValue = m_module->m_typeMgr.getStdType (StdTypeKind_ObjectPtr)->getZeroValue ();

	return createClosurePropertyPtr (thunkProperty, nullValue, dstPtrType, resultValue);
}

bool
Cast_PropertyPtr_Thin2Fat::llvmCast_DirectThunkSimpleClosure (
	Property* prop,
	const Value& simpleClosureObjValue,
	PropertyPtrType* dstPtrType,
	Value* resultValue
	)
{
	Type* thisArgType = prop->getType ()->getThisArgType ();
	NamedType* thisTargetType = prop->getType ()->getThisTargetType ();

	Value thisArgValue;
	bool result = m_module->m_operatorMgr.castOperator (simpleClosureObjValue, thisArgType, &thisArgValue);
	if (!result)
		return false;

	Property* thunkProperty = m_module->m_functionMgr.getDirectThunkProperty (
		prop,
		m_module->m_typeMgr.getMemberPropertyType (thisTargetType, dstPtrType->getTargetType ())
		);

	return createClosurePropertyPtr (thunkProperty, thisArgValue, dstPtrType, resultValue);
}

bool
Cast_PropertyPtr_Thin2Fat::llvmCast_FullClosure (
	StorageKind storageKind,
	const Value& opValue,
	PropertyType* srcPropertyType,
	PropertyPtrType* dstPtrType,
	Value* resultValue
	)
{
	Value closureValue;
	bool result = m_module->m_operatorMgr.createClosureObject (
		storageKind,
		opValue,
		dstPtrType->getTargetType (),
		dstPtrType->getPtrTypeKind () == PropertyPtrTypeKind_Weak,
		&closureValue
		);

	if (!result)
		return false;

	ASSERT (isClassPtrType (closureValue.getType (), ClassTypeKind_PropertyClosure));

	PropertyClosureClassType* closureType = (PropertyClosureClassType*) ((ClassPtrType*) closureValue.getType ())->getTargetType ();
	return createClosurePropertyPtr (closureType->getThunkProperty (), closureValue, dstPtrType, resultValue);
}

bool
Cast_PropertyPtr_Thin2Fat::createClosurePropertyPtr (
	Property* prop,
	const Value& closureValue,
	PropertyPtrType* ptrType,
	Value* resultValue
	)
{
	Value thinPtrValue;
	bool result = m_module->m_operatorMgr.getPropertyThinPtr (prop, NULL, &thinPtrValue);
	if (!result)
		return false;

	m_module->m_llvmIrBuilder.createClosurePropertyPtr (thinPtrValue, closureValue, ptrType, resultValue);
	return true;
}

//.............................................................................

bool
Cast_PropertyPtr_Weak2Normal::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlagKind_PropertyPtr);
	ASSERT (type->getTypeKind () == TypeKind_PropertyPtr && ((PropertyPtrType*) type)->getPtrTypeKind () == PropertyPtrTypeKind_Normal);

	BasicBlock* initialBlock = m_module->m_controlFlowMgr.getCurrentBlock ();
	BasicBlock* strengthenBlock = m_module->m_controlFlowMgr.createBlock ("strengthen");
	BasicBlock* aliveBlock = m_module->m_controlFlowMgr.createBlock ("alive");
	BasicBlock* deadBlock = m_module->m_controlFlowMgr.createBlock ("dead");
	BasicBlock* phiBlock = m_module->m_controlFlowMgr.createBlock ("phi");

	Type* closureType = m_module->getSimpleType (StdTypeKind_ObjectPtr);
	Value nullClosureValue = closureType->getZeroValue ();

	Value closureValue;
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 1, closureType, &closureValue);

	Value cmpValue;
	m_module->m_operatorMgr.binaryOperator (BinOpKind_Ne, closureValue, nullClosureValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, strengthenBlock, phiBlock);

	Function* strengthenFunction = m_module->m_functionMgr.getStdFunction (StdFuncKind_StrengthenClassPtr);

	Value strengthenedClosureValue;
	m_module->m_llvmIrBuilder.createCall (
		strengthenFunction,
		strengthenFunction->getType (),
		closureValue,
		&strengthenedClosureValue
		);

	m_module->m_operatorMgr.binaryOperator (BinOpKind_Ne, strengthenedClosureValue, nullClosureValue, &cmpValue);
	m_module->m_controlFlowMgr.conditionalJump (cmpValue, aliveBlock, deadBlock);
	m_module->m_controlFlowMgr.follow (phiBlock);

	m_module->m_controlFlowMgr.setCurrentBlock (deadBlock);
	m_module->m_controlFlowMgr.follow (phiBlock);

	Value valueArray [3] =
	{
		opValue,
		opValue,
		opValue.getType ()->getZeroValue ()
	};

	BasicBlock* blockArray [3] =
	{
		initialBlock,
		aliveBlock,
		deadBlock
	};

	Value intermediateValue;
	m_module->m_llvmIrBuilder.createPhi (valueArray, blockArray, 3, &intermediateValue);

	PropertyPtrType* intermediateType = ((PropertyPtrType*) opValue.getType ())->getUnWeakPtrType ();
	intermediateValue.overrideType (intermediateType);
	return m_module->m_operatorMgr.castOperator (intermediateValue, type, resultValue);}

//.............................................................................

bool
Cast_PropertyPtr_Thin2Thin::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_PropertyPtr);
	ASSERT (type->getTypeKind () == TypeKind_PropertyPtr);

	if (opValue.getClosure ())
	{
		err::setFormatStringError ("cannot create thin property pointer to a closure");
		return false;
	}

	if (opValue.getValueKind () != ValueKind_Property)
	{
		err::setFormatStringError ("can only create thin pointer thunk to a property, not a property pointer");
		return false;
	}

	PropertyPtrType* ptrType = (PropertyPtrType*) type;
	PropertyType* targetType = ptrType->getTargetType ();
	Property* prop = opValue.getProperty ();

	if (prop->getType ()->cmp (targetType) == 0)
		return m_module->m_operatorMgr.getPropertyThinPtr (prop, NULL, ptrType, resultValue);

	if (prop->getFlags () & PropertyTypeFlagKind_Bindable)
	{
		err::setFormatStringError ("bindable properties are not supported yet");
		return false;
	}

	Property* thunkProperty = m_module->m_functionMgr.getDirectThunkProperty (prop, targetType);
	return m_module->m_operatorMgr.getPropertyThinPtr (thunkProperty, NULL, ptrType, resultValue);
}

//.............................................................................
/*
bool
Cast_PropertyPtr_Thin2Weak::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	#pragma AXL_TODO ("will only work for simple closures. redesign Thin2Normal to support 'weak'")

	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlagKind_PropertyPtr);
	ASSERT (type->getTypeKind () == TypeKind_PropertyPtr);

	if (opValue.getClosure () && !opValue.getClosure ()->isSimpleClosure ())
	{
		err::setFormatStringError ("full weak closures are not implemented yet");
		return false;
	}

	PropertyPtrType* intermediateType = ((PropertyPtrType*) type)->getTargetType ()->getPropertyPtrType (PropertyPtrTypeKind_Normal);
	bool result = m_module->m_operatorMgr.castOperator (storageKind, opValue, intermediateType, resultValue);
	if (!result)
		return false;

	resultValue->overrideType (type);
	return true;
}
*/

//.............................................................................

Cast_PropertyPtr::Cast_PropertyPtr ()
{
	memset (m_operatorTable, 0, sizeof (m_operatorTable));

	m_operatorTable [PropertyPtrTypeKind_Normal] [PropertyPtrTypeKind_Normal] = &m_fromFat;
	m_operatorTable [PropertyPtrTypeKind_Normal] [PropertyPtrTypeKind_Weak]   = &m_fromFat;
	m_operatorTable [PropertyPtrTypeKind_Weak] [PropertyPtrTypeKind_Normal]   = &m_weak2Normal;
	m_operatorTable [PropertyPtrTypeKind_Weak] [PropertyPtrTypeKind_Weak]     = &m_fromFat;
	m_operatorTable [PropertyPtrTypeKind_Thin] [PropertyPtrTypeKind_Normal]   = &m_thin2Fat;
	m_operatorTable [PropertyPtrTypeKind_Thin] [PropertyPtrTypeKind_Weak]     = &m_thin2Fat;
	m_operatorTable [PropertyPtrTypeKind_Thin] [PropertyPtrTypeKind_Thin]     = &m_thin2Thin;
}

CastOperator*
Cast_PropertyPtr::getCastOperator (
	const Value& opValue,
	Type* type
	)
{
	ASSERT (type->getTypeKind () == TypeKind_PropertyPtr);

	PropertyPtrType* dstPtrType = (PropertyPtrType*) type;
	PropertyPtrTypeKind dstPtrTypeKind = dstPtrType->getPtrTypeKind ();
	ASSERT ((size_t) dstPtrTypeKind < PropertyPtrTypeKind__Count);

	TypeKind srcTypeKind = opValue.getType ()->getTypeKind ();
	switch (srcTypeKind)
	{
	case TypeKind_DataPtr:
		return &m_fromDataPtr;

	case TypeKind_PropertyPtr:
		{
		PropertyPtrTypeKind srcPtrTypeKind = ((PropertyPtrType*) opValue.getType ())->getPtrTypeKind ();
		ASSERT ((size_t) srcPtrTypeKind < PropertyPtrTypeKind__Count);

		return m_operatorTable [srcPtrTypeKind] [dstPtrTypeKind];
		}

	default:
		return NULL;
	};
}

//.............................................................................

CastKind
Cast_PropertyRef::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	ASSERT (type->getTypeKind () == TypeKind_PropertyRef);

	Type* intermediateSrcType = m_module->m_operatorMgr.getUnaryOperatorResultType (UnOpKind_Addr, opValue);
	if (!intermediateSrcType)
		return CastKind_None;

	PropertyPtrType* ptrType = (PropertyPtrType*) type;
	PropertyPtrType* intermediateDstType = ptrType->getTargetType ()->getPropertyPtrType (
		TypeKind_PropertyPtr,
		ptrType->getPtrTypeKind (),
		ptrType->getFlags ()
		);

	return m_module->m_operatorMgr.getCastKind (intermediateSrcType, intermediateDstType);
}

bool
Cast_PropertyRef::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (type->getTypeKind () == TypeKind_PropertyRef);

	PropertyPtrType* ptrType = (PropertyPtrType*) type;
	PropertyPtrType* intermediateType = ptrType->getTargetType ()->getPropertyPtrType (
		TypeKind_PropertyPtr,
		ptrType->getPtrTypeKind (),
		ptrType->getFlags ()
		);

	Value intermediateValue;

	return
		m_module->m_operatorMgr.unaryOperator (UnOpKind_Addr, opValue, &intermediateValue) &&
		m_module->m_operatorMgr.castOperator (&intermediateValue, intermediateType) &&
		m_module->m_operatorMgr.unaryOperator (UnOpKind_Indir, intermediateValue, resultValue);
}

//.............................................................................

} // namespace jnc {
