#include "pch.h"
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

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
	
	Closure* closure = value->createClosure ();
	closure->insertThisArgValue (thisValue);
	return true;
}

bool
OperatorMgr::getThisValue (Value* value)
{
	Value thisValue = m_module->m_functionMgr.getThisValue ();
	if (!thisValue)
	{
		err::setFormatStringError ("function '%s' has no 'this' pointer", m_module->m_functionMgr.getCurrentFunction ()->m_tag.sz ());
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
		err::setFormatStringError ("function '%s' has no 'this' pointer", m_module->m_functionMgr.getCurrentFunction ()->m_tag.sz ());
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
OperatorMgr::checkAccess (ModuleItemDecl* decl)
{
	if (decl->getAccessKind () != AccessKind_Public &&
		m_module->m_namespaceMgr.getAccessKind (decl->getParentNamespace ()) == AccessKind_Public)
	{
		err::setFormatStringError ("'%s' is protected", decl->getQualifiedName ().sz ());
		return false;
	}

	return true;
}

bool
OperatorMgr::getNamespaceMemberType (
	Namespace* nspace,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	MemberCoord coord;
	ModuleItem* item = nspace->findItemTraverse (name, &coord, TraverseKind_NoParentNamespace);
	if (!item)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, nspace->getQualifiedName ().sz ());
		return false;
	}

	bool result = true;
	ModuleItemDecl* decl = NULL;

	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		resultValue->setNamespace ((GlobalNamespace*) item);
		decl = (GlobalNamespace*) item;
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*) item)->getType ();
		result = checkAccess ((Typedef*) item);
		// and fall through

	case ModuleItemKind_Type:
		if (!(((Type*) item)->getTypeKindFlags () & TypeKindFlag_Named))
		{
			err::setFormatStringError ("'%s' cannot be used as expression", ((Type*) item)->getTypeString ().sz ());
			return false;
		}

		resultValue->setNamespace ((NamedType*) item);
		decl = (NamedType*) item;
		break;

	case ModuleItemKind_Alias:
		resultValue->setType (((Alias*) item)->getType ());
		decl = (Alias*) item;
		break;

	case ModuleItemKind_Variable:
		resultValue->setType (((Variable*) item)->getType ()->getDataPtrType (TypeKind_DataRef, DataPtrTypeKind_Lean));
		decl = (Variable*) item;
		break;

	case ModuleItemKind_Function:
		resultValue->setFunctionTypeOverload (((Function*) item)->getTypeOverload ());
		if (((Function*) item)->isMember ())
			result = createMemberClosure (resultValue);

		decl = (Function*) item;
		break;

	case ModuleItemKind_Property:
		resultValue->setType (((Property*) item)->getType ()->getPropertyPtrType (TypeKind_PropertyRef, PropertyPtrTypeKind_Thin));
		if (((Property*) item)->isMember ())
			result = createMemberClosure (resultValue);

		decl = (Property*) item;
		break;

	case ModuleItemKind_EnumConst:
		resultValue->setType (((EnumConst*) item)->getParentEnumType ());
		decl = (EnumConst*) item;
		break;

	case ModuleItemKind_StructField:
		resultValue->setField ((StructField*) item, coord.m_offset);
		decl = (StructField*) item;
		break;

	default:
		err::setFormatStringError ("'%s.%s' cannot be used as expression", nspace->getQualifiedName ().sz (), name);
		return false;
	};

	if (!result)
		return false;
	
	ASSERT (decl);
	return checkAccess (decl);
}

bool
OperatorMgr::getNamespaceMember (
	Namespace* nspace,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	ModuleItem* item = nspace->findItemTraverse (name, NULL, TraverseKind_NoParentNamespace);
	if (!item)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, nspace->getQualifiedName ().sz ());
		return false;
	}

	bool result = true;
	ModuleItemDecl* decl = NULL;
	Function* function;

	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		resultValue->setNamespace ((GlobalNamespace*) item);
		decl = (GlobalNamespace*) item;
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*) item)->getType ();
		result = checkAccess ((Typedef*) item);
		// and fall through

	case ModuleItemKind_Type:
		if (!(((Type*) item)->getTypeKindFlags () & TypeKindFlag_Named))
		{
			err::setFormatStringError ("'%s' cannot be used as expression", ((Type*) item)->getTypeString ().sz ());
			return false;
		}

		resultValue->setNamespace ((NamedType*) item);
		decl = (NamedType*) item;
		break;

	case ModuleItemKind_Alias:
		result = evaluateAlias (
			(Alias*) item,
			((Alias*) item)->getInitializer (),
			resultValue
			);
		decl = (Alias*) item;
		break;

	case ModuleItemKind_Variable:
		resultValue->setVariable ((Variable*) item);
		decl = (Variable*) item;
		break;

	case ModuleItemKind_Function:
		function = (Function*) item;

		if (function->isVirtual ())
		{
			if (function->getStorageKind () == StorageKind_Abstract)
			{
				err::setFormatStringError ("'%s' is abstract", function->m_tag.sz ());
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

		decl = function;
		break;

	case ModuleItemKind_Property:
		resultValue->setProperty ((Property*) item);
		if (((Property*) item)->isMember ())
			result = createMemberClosure (resultValue);

		decl = (Property*) item;
		break;

	case ModuleItemKind_EnumConst:
		result = ((EnumConst*) item)->getParentEnumType ()->ensureLayout ();
		if (!result)
			return false;

		resultValue->setConstInt64 (
			((EnumConst*) item)->getValue (),
			((EnumConst*) item)->getParentEnumType ()
			);
		decl = (EnumConst*) item;
		break;

	default:
		err::setFormatStringError ("'%s.%s' cannot be used as expression", nspace->getQualifiedName ().sz (), name);
		return false;
	};

	ASSERT (decl);
	return checkAccess (decl);
}

bool
OperatorMgr::getNamedTypeMemberType (
	const Value& opValue,
	NamedType* namedType,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	MemberCoord coord;
	ModuleItem* member = namedType->findItemTraverse (name, &coord, TraverseKind_NoParentNamespace);
	if (!member)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, namedType->getTypeString ().sz ());
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
	const sl::StringRef& name,
	Value* resultValue
	)
{
	MemberCoord coord;
	ModuleItem* member = namedType->findItemTraverse (name, &coord, TraverseKind_NoParentNamespace);
	if (!member)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, namedType->getTypeString ().sz ());
		return false;
	}

	ModuleItemDecl* decl = NULL; 
	ModuleItemKind memberKind = member->getItemKind ();
	switch (memberKind)
	{
	case ModuleItemKind_Namespace:
		resultValue->setNamespace ((GlobalNamespace*) member);
		decl = (GlobalNamespace*) member;
		break;

	case ModuleItemKind_StructField:
		return 
			checkAccess ((StructField*) member) &&
			getField (opValue, (StructField*) member, &coord, resultValue);

	case ModuleItemKind_Function:
		resultValue->setFunction ((Function*) member);
		decl = (Function*) member;
		break;

	case ModuleItemKind_Property:
		resultValue->setProperty ((Property*) member);
		decl = (Property*) member;
		break;

	case ModuleItemKind_Alias:
		return 
			checkAccess ((Alias*) member) &&
			evaluateAlias (
				(Alias*) member,
				opValue,
				((Alias*) member)->getInitializer (),
				resultValue
				);

	default:
		err::setFormatStringError ("invalid member kind");
		return false;
	}

	if (decl->getStorageKind () == StorageKind_Static)
		return true;

	AXL_TODO ("remove explicit addr operator and instead allow implicit cast named_type& -> named_type*")

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
	
	Closure* closure = resultValue->createClosure ();
	closure->insertThisArgValue (thisArgValue);
	return true;
}

bool
OperatorMgr::getEnumTypeMemberType (
	const Value& opValue,
	EnumType* enumType,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	ModuleItem* member = enumType->findItem (name);
	if (!member)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, enumType->getTypeString ().sz ());
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
	const sl::StringRef& name,
	Value* resultValue
	)
{
	ModuleItem* member = enumType->findItem (name);
	if (!member)
	{
		err::setFormatStringError ("'%s' is not a member of '%s'", name, enumType->getTypeString ().sz ());
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
	const sl::StringRef& name,
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
		err::setFormatStringError ("member operator cannot be applied to '%s'", type->getTypeString ().sz ());
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
			err::setFormatStringError ("indexed member operator cannot be applied to '%s'", type->getTypeString ().sz ());
			return false;
		}

	case TypeKind_ClassRef:
		type = ((ClassPtrType*) type)->getTargetType ();
		field = ((ClassType*) type)->getFieldByIndex (index);
		return field && getClassField (opValue, field, NULL, resultValue);

	default:
		err::setFormatStringError ("indexed member operator cannot be applied to '%s'", type->getTypeString ().sz ());
		return false;
	}
}

bool
OperatorMgr::getLibraryMember (
	DynamicLibNamespace* library,
	Closure* closure,
	const sl::StringRef& name,
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
	const char* namePtr = function->getName (); // make sure name pointer stays valid (points to function, not token string)

	Value argValueArray [] = 
	{
		closure->getThisArgValue (),
		Value (index, m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT)),
		Value (&namePtr, m_module->m_typeMgr.getStdType (StdType_ByteConstPtr)),
	};

	m_module->m_llvmIrBuilder.createBitCast (
		argValueArray [0],
		((ClassType*) m_module->m_typeMgr.getStdType (StdType_DynamicLib))->getClassPtrType (),
		&argValueArray [0]
		);

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope);

	Value ptrValue;

	Function* getterFunction = m_module->m_functionMgr.getStdFunction (StdFunc_TryLazyGetDynamicLibFunction);
	FunctionType* getterFunctionType = getterFunction->getType ();

	m_module->m_llvmIrBuilder.createCall (
		getterFunction,
		getterFunctionType,
		argValueArray,
		countof (argValueArray),
		&ptrValue
		);

	result = m_module->m_controlFlowMgr.throwExceptionIf (ptrValue, getterFunctionType);
	ASSERT (result);

	Type* resultType = function->getType ()->getFunctionPtrType (FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe);
	m_module->m_llvmIrBuilder.createBitCast (ptrValue, resultType, resultValue);
	return true;
}

bool
OperatorMgr::memberOperator (
	const Value& rawOpValue,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	if (rawOpValue.getValueKind () == ValueKind_Namespace)
	{
		Namespace* nspace = rawOpValue.getNamespace ();
		return nspace->getNamespaceKind () == NamespaceKind_DynamicLib ?
			getLibraryMember ((DynamicLibNamespace*) nspace, rawOpValue.getClosure (), name, resultValue) :
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
		err::setFormatStringError ("member operator cannot be applied to '%s'", type->getTypeString ().sz ());
		return false;
	}
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

} // namespace ct
} // namespace jnc


