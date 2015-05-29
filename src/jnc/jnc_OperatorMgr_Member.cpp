#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
OperatorMgr::createMemberClosure (Value* value)
{
	Value thisValue;

	bool result = value->getValueKind () == ValueKind_Type ?
		getThisValueType (&thisValue) :
		getThisValue (&thisValue);

	if (!result)
		return false;

	value->insertToClosureHead (thisValue);
	return true;
}

bool
OperatorMgr::getThisValue (Value* value)
{
	Value thisValue = m_module->m_functionMgr.getThisValue ();
	if (!thisValue)
	{
		err::setFormatStringError ("function '%s' has no 'this' pointer", m_module->m_functionMgr.getCurrentFunction ()->m_tag.cc ());
		return false;
	}

	if (isClassPtrType (thisValue.getType (), ClassTypeKind_Reactor))
	{
		ClassType* classType = ((ClassPtrType*) thisValue.getType ())->getTargetType ();
		ReactorClassType* reactorType = (ReactorClassType*) classType;
		if (reactorType->getField (ReactorFieldKind_Parent))
		{
			StructField* parentField = reactorType->getField (ReactorFieldKind_Parent);
			bool result =
				m_module->m_operatorMgr.getField (&thisValue, parentField) &&
				m_module->m_operatorMgr.prepareOperand (&thisValue);

			if (!result)
				return false;
		}
	}

	*value = thisValue;
	return true;
}

bool
OperatorMgr::getThisValueType (Value* value)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	if (!function->isMember ())
	{
		err::setFormatStringError ("function '%s' has no 'this' pointer", m_module->m_functionMgr.getCurrentFunction ()->m_tag.cc ());
		return false;
	}

	Type* thisType = function->getThisType ();
	if (isClassPtrType (thisType, ClassTypeKind_Reactor))
	{
		ClassType* classType = ((ClassPtrType*) thisType)->getTargetType ();
		ReactorClassType* reactorType = (ReactorClassType*) classType;
		if (reactorType->getField (ReactorFieldKind_Parent))
		{
			StructField* parentField = reactorType->getField (ReactorFieldKind_Parent);
			thisType = parentField->getType ();
		}
	}

	value->setType (thisType);
	return true;
}

bool
OperatorMgr::getNamespaceMemberType (
	Namespace* nspace,
	const char* name,
	Value* resultValue
	)
{
	MemberCoord coord;
	ModuleItem* item = nspace->findItemTraverse (name, &coord, TraverseKind_NoParentNamespace);
	if (!item)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, nspace->getQualifiedName ().cc ());
		return false;
	}

	ModuleItemDecl* decl = item->getItemDecl ();
	ASSERT (decl);

	if (decl->getAccessKind () != AccessKind_Public &&
		m_module->m_namespaceMgr.getAccessKind (nspace) == AccessKind_Public)
	{
		err::setFormatStringError ("'%s.%s' is protected", nspace->getQualifiedName ().cc (), name);
		return false;
	}

	bool result = true;

	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		resultValue->setNamespace ((GlobalNamespace*) item);
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*) item)->getType ();
		// and fall through

	case ModuleItemKind_Type:
		if (!(((Type*) item)->getTypeKindFlags () & TypeKindFlag_Named))
		{
			err::setFormatStringError ("'%s' cannot be used as expression", ((Type*) item)->getTypeString ().cc ());
			return false;
		}

		resultValue->setNamespace ((NamedType*) item);
		break;

	case ModuleItemKind_Alias:
		resultValue->setType (((Alias*) item)->getType ());
		break;

	case ModuleItemKind_Variable:
		resultValue->setType (((Variable*) item)->getType ()->getDataPtrType (TypeKind_DataRef, DataPtrTypeKind_Lean));
		break;

	case ModuleItemKind_Function:
		resultValue->setFunctionTypeOverload (((Function*) item)->getTypeOverload ());
		if (((Function*) item)->isMember ())
			result = createMemberClosure (resultValue);
		break;

	case ModuleItemKind_Property:
		resultValue->setType (((Property*) item)->getType ()->getPropertyPtrType (TypeKind_PropertyRef, PropertyPtrTypeKind_Thin));
		if (((Property*) item)->isMember ())
			result = createMemberClosure (resultValue);
		break;

	case ModuleItemKind_EnumConst:
		resultValue->setType (((EnumConst*) item)->getParentEnumType ());
		break;

	case ModuleItemKind_StructField:
		resultValue->setField ((StructField*) item, coord.m_offset);
		break;

	default:
		result = false;
	};

	if (!result)
	{
		err::setFormatStringError ("'%s.%s' cannot be used as expression", nspace->getQualifiedName ().cc (), name);
		return false;
	}

	return true;
}

bool
OperatorMgr::getNamespaceMember (
	Namespace* nspace,
	const char* name,
	Value* resultValue
	)
{
	ModuleItem* item = nspace->findItemTraverse (name, NULL, TraverseKind_NoParentNamespace);
	if (!item)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, nspace->getQualifiedName ().cc ());
		return false;
	}

	ModuleItemDecl* decl = item->getItemDecl ();
	ASSERT (decl);

	if (decl->getAccessKind () != AccessKind_Public &&
		m_module->m_namespaceMgr.getAccessKind (nspace) == AccessKind_Public)
	{
		err::setFormatStringError ("'%s.%s' is protected", nspace->getQualifiedName ().cc (), name);
		return false;
	}

	bool result = true;

	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		resultValue->setNamespace ((GlobalNamespace*) item);
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*) item)->getType ();
		// and fall through

	case ModuleItemKind_Type:
		if (!(((Type*) item)->getTypeKindFlags () & TypeKindFlag_Named))
		{
			err::setFormatStringError ("'%s' cannot be used as expression", ((Type*) item)->getTypeString ().cc ());
			return false;
		}

		resultValue->setNamespace ((NamedType*) item);
		break;

	case ModuleItemKind_Alias:
		return evaluateAlias (
			item->getItemDecl (),
			((Alias*) item)->getInitializer (),
			resultValue
			);

	case ModuleItemKind_Variable:
		resultValue->setVariable ((Variable*) item);
		break;

	case ModuleItemKind_Function:
		Function* function;
		function = (Function*) item;

		if (function->isVirtual ())
		{
			if (function->getStorageKind () == StorageKind_Abstract)
			{
				err::setFormatStringError ("'%s' is abstract", function->m_tag.cc ());
				return false;
			}

			resultValue->setLlvmValue (
				function->getLlvmFunction (),
				function->getType ()->getFunctionPtrType (FunctionPtrTypeKind_Thin)
				);

			result = createMemberClosure (resultValue);
		}
		else if (function->isMember ())
		{
			resultValue->setFunction (function);
			result = createMemberClosure (resultValue);
		}
		else
		{
			resultValue->setFunction (function);
		}

		break;

	case ModuleItemKind_Property:
		resultValue->setProperty ((Property*) item);
		if (((Property*) item)->isMember ())
			result = false; 

		break;

	case ModuleItemKind_EnumConst:
		result = ((EnumConst*) item)->getParentEnumType ()->ensureLayout ();
		if (!result)
			return false;

		resultValue->setConstInt64 (
			((EnumConst*) item)->getValue (),
			((EnumConst*) item)->getParentEnumType ()
			);
		break;

	default:
		result = false;
	};

	if (!result)
	{
		err::setFormatStringError ("'%s.%s' cannot be used as expression", nspace->getQualifiedName ().cc (), name);
		return false;
	}

	return true;
}

bool
OperatorMgr::getNamedTypeMemberType (
	const Value& opValue,
	NamedType* namedType,
	const char* name,
	Value* resultValue
	)
{
	MemberCoord coord;
	ModuleItem* member = namedType->findItemTraverse (name, &coord, TraverseKind_NoParentNamespace);
	if (!member)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, namedType->getTypeString ().cc ());
		return false;
	}

	ModuleItemKind memberKind = member->getItemKind ();
	switch (memberKind)
	{
	case ModuleItemKind_Namespace:
		resultValue->setNamespace ((GlobalNamespace*) member);
		break;

	case ModuleItemKind_StructField:
		{
		StructField* field = (StructField*) member;
		size_t baseOffset = 0;
		if (opValue.getValueKind () == ValueKind_Field)
			baseOffset = opValue.getFieldOffset ();

		if (!(opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr))
		{
			resultValue->setField (field, baseOffset); 
			break;
		}

		DataPtrType* ptrType = (DataPtrType*) opValue.getType ();
		DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

		Type* resultType = field->getType ()->getDataPtrType (
			TypeKind_DataRef,
			ptrTypeKind == DataPtrTypeKind_Thin ? DataPtrTypeKind_Thin : DataPtrTypeKind_Lean,
			opValue.getType ()->getFlags ()
			);

		resultValue->setField (field, resultType, baseOffset); 
		break;
		}

	case ModuleItemKind_Function:
		resultValue->setType (((Function*) member)->getType ()->getShortType ()->getFunctionPtrType (
			TypeKind_FunctionRef,
			FunctionPtrTypeKind_Thin
			));
		break;

	case ModuleItemKind_Property:
		resultValue->setType (((Property*) member)->getType ()->getShortType ()->getPropertyPtrType (
			TypeKind_PropertyRef,
			PropertyPtrTypeKind_Thin
			));
		break;

	default:
		err::setFormatStringError ("invalid member kind '%s'", getModuleItemKindString (memberKind));
		return false;
	}

	return true;
}

bool
OperatorMgr::getNamedTypeMember (
	const Value& opValue,
	NamedType* namedType,
	const char* name,
	Value* resultValue
	)
{
	MemberCoord coord;
	ModuleItem* member = namedType->findItemTraverse (name, &coord, TraverseKind_NoParentNamespace);
	if (!member)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, namedType->getTypeString ().cc ());
		return false;
	}

	ModuleItemDecl* decl = member->getItemDecl ();
	ASSERT (decl);

	if (decl->getAccessKind () != AccessKind_Public &&
		m_module->m_namespaceMgr.getAccessKind (coord.m_type) == AccessKind_Public)
	{
		err::setFormatStringError ("'%s.%s' is protected", coord.m_type->getQualifiedName ().cc (), name);
		return false;
	}

	ModuleItemKind memberKind = member->getItemKind ();
	switch (memberKind)
	{
	case ModuleItemKind_Namespace:
		resultValue->setNamespace ((GlobalNamespace*) member);
		break;

	case ModuleItemKind_StructField:
		return getField (opValue, (StructField*) member, &coord, resultValue);

	case ModuleItemKind_Function:
		resultValue->setFunction ((Function*) member);
		break;

	case ModuleItemKind_Property:
		resultValue->setProperty ((Property*) member);
		break;

	default:
		err::setFormatStringError ("invalid member kind");
		return false;
	}

	if (decl->getStorageKind () == StorageKind_Static)
		return true;

	#pragma AXL_TODO ("remove explicit addr operator and instead allow implicit cast named_type& -> named_type*")

	Value thisArgValue = opValue;
	if (namedType->getTypeKind () != TypeKind_Class)
	{
		bool result = unaryOperator (UnOpKind_Addr, &thisArgValue);
		if (!result)
			return false;
	}

	if (isClassType (namedType, ClassTypeKind_Multicast))
	{
		ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_ClassPtr);
		if ((member->getFlags () & MulticastMethodFlag_InaccessibleViaEventPtr) &&
			((ClassPtrType*) opValue.getType ())->isEventPtrType ())
		{
			err::setFormatStringError ("'%s' is inaccessible via 'event' pointer", name);
			return false;
		}
	}

	resultValue->insertToClosureHead (thisArgValue);
	return true;
}

bool
OperatorMgr::getEnumTypeMemberType (
	const Value& opValue,
	EnumType* enumType,
	const char* name,
	Value* resultValue
	)
{
	ModuleItem* member = enumType->findItem (name);
	if (!member)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, enumType->getTypeString ().cc ());
		return false;
	}

	Type* resultType = (enumType->getFlags () & EnumTypeFlag_BitFlag) ? 
		enumType :
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);

	resultValue->setType (resultType);
	return true;
}

bool
OperatorMgr::getEnumTypeMember (
	const Value& opValue,
	EnumType* enumType,
	const char* name,
	Value* resultValue
	)
{
	ModuleItem* member = enumType->findItem (name);
	if (!member)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, enumType->getTypeString ().cc ());
		return false;
	}

	ASSERT (member->getItemKind () == ModuleItemKind_EnumConst);
	EnumConst* enumConst = (EnumConst*) member;
	Value memberValue (enumConst->getValue (), enumType);
	BinOpKind binOpKind = (enumType->getFlags () & EnumTypeFlag_BitFlag) ? BinOpKind_BwAnd : BinOpKind_Eq;

	return binaryOperator (
		binOpKind,
		opValue, 
		memberValue,
		resultValue
		);
}

bool
OperatorMgr::getMemberOperatorResultType (
	const Value& rawOpValue,
	const char* name,
	Value* resultValue
	)
{
	if (rawOpValue.getValueKind () == ValueKind_Namespace)
		return getNamespaceMemberType (rawOpValue.getNamespace (), name, resultValue);

	Value opValue;
	prepareOperandType (rawOpValue, &opValue, OpFlag_KeepDataRef | OpFlag_KeepEnum);

	Type* type = opValue.getType ();
	if (type->getTypeKind () == TypeKind_DataRef)
		type = ((DataPtrType*) type)->getTargetType ();

	if (type->getTypeKind () == TypeKind_DataPtr)
	{
		type = ((DataPtrType*) type)->getTargetType ();

		bool result = getUnaryOperatorResultType (UnOpKind_Indir, &opValue);
		if (!result)
			return false;
	}

	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_Struct:
	case TypeKind_Union:
		return getNamedTypeMemberType (opValue, (NamedType*) type, name, resultValue);

	case TypeKind_ClassPtr:
		prepareOperandType (&opValue);
		return getNamedTypeMemberType (opValue, ((ClassPtrType*) type)->getTargetType (), name, resultValue);

	case TypeKind_Enum:
		prepareOperandType (&opValue);
		return getEnumTypeMemberType (opValue, (EnumType*) type, name, resultValue);

	default:
		err::setFormatStringError ("member operator cannot be applied to '%s'", type->getTypeString ().cc ());
		return false;
	}
}

bool
OperatorMgr::memberOperator (
	const Value& rawOpValue,
	size_t index,
	Value* resultValue
	)
{
	Value opValue;
	bool result = prepareOperand (rawOpValue, &opValue, OpFlag_KeepDataRef | OpFlag_KeepClassRef);
	if (!result)
		return false;

	Type* type = opValue.getType ();
	TypeKind typeKind = type->getTypeKind ();

	StructField* field;

	switch (typeKind)
	{
	case TypeKind_DataRef:
		type = ((DataPtrType*) type)->getTargetType ();
		typeKind = type->getTypeKind ();
		switch (typeKind)
		{
		case TypeKind_Array:
			return binaryOperator (
				BinOpKind_Idx, 
				opValue, 
				Value (index, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)), 
				resultValue
				);

		case TypeKind_Struct:
			field = ((StructType*) type)->getFieldByIndex (index);
			return field && getStructField (opValue, field, NULL, resultValue);

		case TypeKind_Union:
			field = ((UnionType*) type)->getFieldByIndex (index);
			return field && getUnionField (opValue, field, resultValue);

		default:
			err::setFormatStringError ("indexed member operator cannot be applied to '%s'", type->getTypeString ().cc ());
			return false;
		}

	case TypeKind_ClassRef:
		type = ((ClassPtrType*) type)->getTargetType ();
		field = ((ClassType*) type)->getFieldByIndex (index);
		return field && getClassField (opValue, field, NULL, resultValue);

	default:
		err::setFormatStringError ("indexed member operator cannot be applied to '%s'", type->getTypeString ().cc ());
		return false;
	}
}

bool
OperatorMgr::getLibraryMember (
	LibraryNamespace* library,
	jnc::Closure* closure,
	const char* name,
	Value* resultValue
	)
{
	ASSERT (closure && closure->isMemberClosure ());

	Value memberValue;
	bool result = getNamespaceMember (library, name, &memberValue);
	if (!result || memberValue.getValueKind () != ValueKind_Function)
		return result;

	Function* function = memberValue.getFunction ();
	size_t index = function->getLibraryTableIndex ();
	name = function->getName (); // make sure name pointer stays valid (points to function, not token string)

	Value argValueArray [] = 
	{
		closure->getThisValue (),
		Value (index, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)),
		Value (&name, m_module->m_typeMgr.getStdType (StdType_ByteConstPtr)),
	};

	m_module->m_llvmIrBuilder.createBitCast (
		argValueArray [0],
		((ClassType*) m_module->m_typeMgr.getStdType (StdType_Library))->getClassPtrType (),
		&argValueArray [0]
		);

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope);

	Value ptrValue;

	if (!(scope->getFlags () & ScopeFlag_CanThrow))
	{
		Function* getterFunction = m_module->m_functionMgr.getStdFunction (StdFunction_LazyGetLibraryFunction);

		m_module->m_llvmIrBuilder.createCall (
			getterFunction,
			getterFunction->getType (),
			argValueArray,
			countof (argValueArray),
			&ptrValue
			);
	}
	else
	{
		Function* getterFunction = m_module->m_functionMgr.getStdFunction (StdFunction_TryLazyGetLibraryFunction);
		FunctionType* getterFunctionType = getterFunction->getType ();

		m_module->m_llvmIrBuilder.createCall (
			getterFunction,
			getterFunctionType,
			argValueArray,
			countof (argValueArray),
			&ptrValue
			);

		bool result = m_module->m_controlFlowMgr.throwIf (ptrValue, getterFunctionType);
		ASSERT (result);
	}

	Type* resultType = function->getType ()->getFunctionPtrType (FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe);
	m_module->m_llvmIrBuilder.createBitCast (ptrValue, resultType, resultValue);
	return true;
}

bool
OperatorMgr::memberOperator (
	const Value& rawOpValue,
	const char* name,
	Value* resultValue
	)
{
	if (rawOpValue.getValueKind () == ValueKind_Namespace)
	{
		Namespace* nspace = rawOpValue.getNamespace ();
		return nspace->getNamespaceKind () == NamespaceKind_Library ?
			getLibraryMember ((LibraryNamespace*) nspace, rawOpValue.getClosure (), name, resultValue) :
			getNamespaceMember (nspace, name, resultValue);
	}

	Value opValue;
	bool result = prepareOperand (rawOpValue, &opValue, OpFlag_KeepDataRef | OpFlag_KeepEnum);
	if (!result)
		return false;

	Type* type = opValue.getType ();

	if (type->getTypeKind () == TypeKind_DataRef)
		type = ((DataPtrType*) type)->getTargetType ();

	if (type->getTypeKind () == TypeKind_DataPtr)
	{
		type = ((DataPtrType*) type)->getTargetType ();

		result = unaryOperator (UnOpKind_Indir, &opValue);
		if (!result)
			return false;
	}

	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_Struct:
	case TypeKind_Union:
		return getNamedTypeMember (opValue, (NamedType*) type, name, resultValue);
		
	case TypeKind_ClassPtr:
		return
			prepareOperand (&opValue) &&
			getNamedTypeMember (opValue, ((ClassPtrType*) type)->getTargetType (), name, resultValue);

	case TypeKind_Enum:
		return
			prepareOperand (&opValue) &&
			getEnumTypeMember (opValue, (EnumType*) type, name, resultValue);

	default:
		err::setFormatStringError ("member operator cannot be applied to '%s'", type->getTypeString ().cc ());
		return false;
	}
}

ClassPtrType*
OperatorMgr::getWeakenOperatorResultType (const Value& opValue)
{
	Type* opType = prepareOperandType (opValue);
	if (opType->getTypeKind () != TypeKind_ClassPtr)
	{
		err::setFormatStringError ("'weak member' operator cannot be applied to '%s'", opType->getTypeString ().cc ());
		return NULL;
	}

	ClassPtrType* resultType = ((ClassPtrType*) opType)->getWeakPtrType ();
	return resultType;
}

bool
OperatorMgr::getWeakenOperatorResultType (
	const Value& opValue,
	Value* resultValue
	)
{
	Type* type = getWeakenOperatorResultType (opValue);
	if (!type)
		return false;

	resultValue->setType (type);
	return true;
}

bool
OperatorMgr::weakenOperator (
	const Value& rawOpValue,
	Value* resultValue
	)
{
	Value opValue;
	bool result = prepareOperand (rawOpValue, &opValue);
	if (!result)
		return false;

	Type* opType = opValue.getType ();
	if (opType->getTypeKind () != TypeKind_ClassPtr)
	{
		err::setFormatStringError ("'weak member' operator cannot be applied to '%s'", opType->getTypeString ().cc ());
		return false;
	}

	ClassPtrType* resultType = ((ClassPtrType*) opType)->getWeakPtrType ();
	resultValue->overrideType (opValue, resultType);
	return true;
}

bool
OperatorMgr::offsetofOperator (
	const Value& value,
	Value* resultValue
	)
{
	if (value.getValueKind () != ValueKind_Field)
	{
		err::setFormatStringError ("'offsetof' can only be applied to fields");
		return false;
	}

	resultValue->setConstSizeT (value.getFieldOffset (), m_module);
	return true;
}

//.............................................................................

} // namespace jnc {


