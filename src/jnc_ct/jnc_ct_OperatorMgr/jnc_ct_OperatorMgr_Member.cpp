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
#include "jnc_ct_ReactorClassType.h"
#include "jnc_ct_UnionType.h"
#include "jnc_ct_DynamicLibNamespace.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Namespace*
OperatorMgr::getValueNamespace(const Value& rawOpValue)
{
	if (rawOpValue.getValueKind() == ValueKind_Namespace)
		return rawOpValue.getNamespace();

	Value opValue;
	prepareOperandType(rawOpValue, &opValue, OpFlag_KeepEnum);

	Type* type = opValue.getType();
	TypeKind typeKind = type->getTypeKind();
	switch (typeKind)
	{
	case TypeKind_DataPtr:
	case TypeKind_DataRef:
		type = ((DataPtrType*)type)->getTargetType();
		break;

	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		type = ((ClassPtrType*)type)->getTargetType();
		break;
	}

	return (type->getTypeKindFlags() & TypeKindFlag_Named) ? (NamedType*)type : NULL;
}

bool
OperatorMgr::memberOperator(
	const Value& rawOpValue,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	bool result;
	Namespace* nspace;
	Field* field;

	ValueKind valueKind = rawOpValue.getValueKind();
	switch (valueKind)
	{
	case ValueKind_Namespace:
		nspace = rawOpValue.getNamespace();
		return nspace->getNamespaceKind() == NamespaceKind_DynamicLib ?
			getLibraryMember((DynamicLibNamespace*)nspace, rawOpValue.getClosure(), name, resultValue) :
			getNamespaceMember(nspace, name, 0, resultValue);

	case ValueKind_Field:
		field = rawOpValue.getField();
		result = field->getType()->ensureLayout();
		if (!result)
			return false;

		if (!(field->getType()->getTypeKindFlags() & TypeKindFlag_Named))
		{
			err::setFormatStringError("member operator cannot be applied to '%s'", field->getType()->getTypeString().sz());
			return false;
		}

		nspace = (NamedType*)field->getType();
		return getNamespaceMember(nspace, name, rawOpValue.getFieldOffset(), resultValue);
	}

	Value opValue;
	result = prepareOperand(rawOpValue, &opValue, OpFlag_KeepDataRef | OpFlag_KeepEnum);
	if (!result)
		return false;

	Type* type = opValue.getType();

	if (type->getTypeKind() == TypeKind_DataRef)
		type = ((DataPtrType*)type)->getTargetType();

	if (type->getTypeKind() == TypeKind_DataPtr)
	{
		result = unaryOperator(UnOpKind_Indir, &opValue);
		if (!result)
			return false;

		ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_DataPtr);
		type = ((DataPtrType*)opValue.getType())->getTargetType();

		result = type->ensureLayout();
		if (!result)
			return false;
	}

	TypeKind typeKind = type->getTypeKind();
	switch (typeKind)
	{
	case TypeKind_Struct:
	case TypeKind_Union:
		return getNamedTypeMember(opValue, (NamedType*)type, name, resultValue);

	case TypeKind_ClassPtr:
		return
			prepareOperand(&opValue) &&
			getNamedTypeMember(opValue, ((ClassPtrType*)type)->getTargetType(), name, resultValue);

	case TypeKind_Enum:
		return
			prepareOperand(&opValue) &&
			getEnumTypeMember(opValue, (EnumType*)type, name, resultValue);

	case TypeKind_Variant:
		return getVariantMember(opValue, name, resultValue);

	default:
		err::setFormatStringError("member operator cannot be applied to '%s'", type->getTypeString().sz());
		return false;
	}
}

bool
OperatorMgr::createMemberClosure(
	Value* value,
	ModuleItemDecl* itemDecl
	)
{
	ValueKind valueKind = value->getValueKind();

	Value thisValue;

	bool result = valueKind == ValueKind_Type || valueKind == ValueKind_FunctionTypeOverload ?
		getThisValueType(&thisValue, itemDecl) :
		getThisValue(&thisValue, itemDecl);

	if (!result)
		return false;

	Closure* closure = value->createClosure();
	closure->insertThisArgValue(thisValue);
	return true;
}

bool
OperatorMgr::getThisValue(
	Value* value,
	ModuleItemDecl* itemDecl
	)
{
	Value thisValue = m_module->m_functionMgr.getThisValue();
	if (!thisValue)
	{
		err::setFormatStringError(
			"function '%s' has no 'this' pointer",
			m_module->m_functionMgr.getCurrentFunction ()->getQualifiedName().sz()
			);

		return false;
	}

	if (!(itemDecl && isReactorClassTypeMember(itemDecl)) &&
		isClassPtrType(thisValue.getType(), ClassTypeKind_Reactor))
	{
		ClassType* classType = ((ClassPtrType*)thisValue.getType())->getTargetType();
		ReactorClassType* reactorType = (ReactorClassType*)classType;
		ClassType* parentType = reactorType->getParentType();

		if (parentType)
			if (!m_module->hasCodeGen())
			{
				thisValue.setType(parentType->getClassPtrType());
			}
			else
			{
				size_t parentOffset = reactorType->getParentOffset();
				ASSERT(parentOffset);

				m_module->m_llvmIrBuilder.createBitCast(thisValue, m_module->m_typeMgr.getStdType(StdType_BytePtr), &thisValue);
				m_module->m_llvmIrBuilder.createGep(thisValue, -parentOffset, NULL, &thisValue);
				m_module->m_llvmIrBuilder.createBitCast(thisValue, parentType->getClassPtrType(), &thisValue);
			}
	}

	*value = thisValue;
	return true;
}

bool
OperatorMgr::getThisValueType(
	Value* value,
	ModuleItemDecl* itemDecl
	)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction();
	if (!function->isMember())
	{
		err::setFormatStringError(
			"function '%s' has no 'this' pointer",
			m_module->m_functionMgr.getCurrentFunction()->getQualifiedName().sz()
			);

		return false;
	}

	Type* thisType = function->getThisType();
	if (!(itemDecl && isReactorClassTypeMember(itemDecl)) &&
		isClassPtrType(thisType, ClassTypeKind_Reactor))
	{
		ClassType* classType = ((ClassPtrType*)thisType)->getTargetType();
		ReactorClassType* reactorType = (ReactorClassType*)classType;
		ClassType* parentType = reactorType->getParentType();

		if (parentType)
			thisType = parentType->getClassPtrType();
	}

	value->setType(thisType);
	return true;
}

bool
OperatorMgr::checkAccess(ModuleItemDecl* decl)
{
	Namespace* nspace = decl->getParentNamespace();
	if (decl->getAccessKind() != AccessKind_Public &&
		m_module->m_namespaceMgr.getAccessKind(nspace) == AccessKind_Public)
	{
		err::setFormatStringError("'%s.%s' is protected", nspace->getQualifiedName().sz(), decl->getName().sz());
		return false;
	}

	return true;
}

void
OperatorMgr::foldDualType(
	const Value& opValue,
	ModuleItemDecl* decl,
	Value* resultValue
	)
{
	Type* type = resultValue->getType();
	ASSERT(isDualType(type));

	Namespace* nspace = decl->getParentNamespace();
	bool isAlien = m_module->m_namespaceMgr.getAccessKind(nspace) == AccessKind_Public;
	bool isConst = (opValue.getType()->getFlags() & PtrTypeFlag_Const) != 0;

	type = m_module->m_typeMgr.foldDualType(type, isAlien, isConst);
	resultValue->overrideType(type);
}

bool
OperatorMgr::getNamespaceMember(
	Namespace* nspace,
	const sl::StringRef& name,
	size_t baseFieldOffset,
	Value* resultValue
	)
{
	bool result;

	FindModuleItemResult findResult = nspace->findDirectChildItemTraverse(name, NULL, TraverseFlag_NoParentNamespace);
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item)
	{
		err::setFormatStringError("'%s' is not a member of '%s'", name.sz(), nspace->getQualifiedName().sz());
		return false;
	}

	ModuleItemDecl* decl = NULL;
	ModuleItem* item = findResult.m_item;
	ModuleItemKind itemKind = item->getItemKind();

	if (itemKind == ModuleItemKind_Alias)
	{
		item = ((Alias*)item)->getTargetItem();
		itemKind = item->getItemKind();
		ASSERT(itemKind != ModuleItemKind_Alias); // should have been resolved at calclayout stage
	}

	switch (itemKind)
	{
	case ModuleItemKind_Namespace:
		resultValue->setNamespace((GlobalNamespace*)item);
		decl = (GlobalNamespace*)item;
		break;

	case ModuleItemKind_Typedef:
		item = ((Typedef*)item)->getType();
		result = checkAccess((Typedef*)item);
		if (!result)
			return false;

		// and fall through

	case ModuleItemKind_Type:
		if (!(((Type*)item)->getTypeKindFlags() & TypeKindFlag_Named))
		{
			err::setFormatStringError("'%s' cannot be used as expression", ((Type*) item)->getTypeString().sz());
			return false;
		}

		resultValue->setNamespace((NamedType*)item);
		decl = (NamedType*)item;
		break;

	case ModuleItemKind_Variable:
		resultValue->setVariable((Variable*)item);
		decl = (Variable*)item;
		break;

	case ModuleItemKind_Function:
		Function* function;
		function = (Function*)item;
		if (function->isVirtual())
		{
			if (function->getStorageKind() == StorageKind_Abstract)
			{
				err::setFormatStringError("'%s' is abstract", function->getQualifiedName().sz());
				return false;
			}

			result = function->getType()->ensureLayout();
			if (!result)
				return false;

			resultValue->setLlvmValue(
				function->getLlvmFunction(),
				function->getType()->getFunctionPtrType(FunctionPtrTypeKind_Thin)
				);

			result = createMemberClosure(resultValue);
			if (!result)
				return false;
		}
		else
		{
			result = resultValue->trySetFunction((Function*)item);
			if (!result)
				return false;

			if (function->isMember())
			{
				result = createMemberClosure(resultValue);
				if (!result)
					return false;
			}
		}

		decl = function;
		break;

	case ModuleItemKind_FunctionOverload:
		resultValue->setFunctionOverload((FunctionOverload*)item);

		if (((FunctionOverload*)item)->getFlags() & FunctionOverloadFlag_HasMembers)
		{
			result = createMemberClosure(resultValue);
			if (!result)
				return false;
		}

		decl = (FunctionOverload*)item;
		break;

	case ModuleItemKind_Property:
		resultValue->setProperty((Property*)item);
		if (((Property*)item)->isMember())
		{
			result = createMemberClosure(resultValue);
			if (!result)
				return false;
		}

		decl = (Property*)item;
		break;

	case ModuleItemKind_EnumConst:
		result = resultValue->trySetEnumConst((EnumConst*)item);
		if (!result)
			return false;

		decl = (EnumConst*)item;
		break;

	case ModuleItemKind_Field:
		if (nspace->getNamespaceKind() != NamespaceKind_Type)
		{
			err::setFormatStringError("'%s.%s' cannot be used as expression", nspace->getQualifiedName().sz(), name.sz());
			return false;
		}

		result = ((NamedType*)nspace)->ensureLayout();
		if (!result)
			return false;

		resultValue->setField((Field*)item, baseFieldOffset);
		decl = (Field*)item;
		break;

	default:
		err::setFormatStringError("'%s.%s' cannot be used as expression", nspace->getQualifiedName().sz(), name.sz());
		return false;
	};

	return finalizeMemberOperator(Value(), decl, resultValue);
}

bool
OperatorMgr::getNamedTypeMember(
	const Value& opValue,
	NamedType* namedType,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	bool result = namedType->ensureLayout();
	if (!result)
		return false;

	MemberCoord coord;
	FindModuleItemResult findResult = namedType->findDirectChildItemTraverse(name, &coord, TraverseFlag_NoParentNamespace);
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item)
	{
		err::setFormatStringError("'%s' is not a member of '%s'", name.sz(), namedType->getTypeString().sz());
		return false;
	}

	ModuleItemDecl* decl = NULL;
	ModuleItem* member = findResult.m_item;
	ModuleItemKind memberKind = member->getItemKind();

	switch (memberKind)
	{
	case ModuleItemKind_Namespace:
		resultValue->setNamespace((GlobalNamespace*)member);
		decl = (GlobalNamespace*)member;
		break;

	case ModuleItemKind_Field:
		if (m_module->m_controlFlowMgr.isEmissionLocked()) // sizeof/countof/offsetof/typeof operators, keep Field
		{
			resultValue->setField((Field*)member, coord.m_offset);
			return true;
		}

		return
			getField(opValue, (Field*)member, &coord, resultValue) &&
			finalizeMemberOperator(opValue, (Field*)member, resultValue);

	case ModuleItemKind_Variable:
		resultValue->setVariable((Variable*)member);
		decl = (Variable*)member;
		break;

	case ModuleItemKind_Function:
		result = resultValue->trySetFunction((Function*)member);
		if (!result)
			return false;

		decl = (Function*)member;
		break;

	case ModuleItemKind_FunctionOverload:
		resultValue->setFunctionOverload((FunctionOverload*)member);
		decl = (FunctionOverload*)member;
		break;

	case ModuleItemKind_Property:
		resultValue->setProperty((Property*)member);
		decl = (Property*)member;
		break;

	default:
		err::setFormatStringError("invalid member kind '%s'", getModuleItemKindString(memberKind));
		return false;
	}

	result = finalizeMemberOperator(opValue, decl, resultValue);
	if (!result)
		return false;

	if (decl->getStorageKind() == StorageKind_Static)
		return true;

	AXL_TODO("remove explicit addr operator and instead allow implicit cast named_type& -> named_type*")

	Value thisArgValue = opValue;
	if (namedType->getTypeKind() != TypeKind_Class)
	{
		result = unaryOperator(UnOpKind_Addr, &thisArgValue);
		if (!result)
			return false;
	}

	if (isClassType(namedType, ClassTypeKind_Multicast))
	{
		ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_ClassPtr);
		if ((member->getFlags() & MulticastMethodFlag_InaccessibleViaEventPtr) &&
			(opValue.getType()->getFlags() & PtrTypeFlag_Event))
		{
			err::setFormatStringError("'%s' is inaccessible via 'event' pointer", name.sz());
			return false;
		}
	}

	Closure* closure = resultValue->createClosure();
	closure->insertThisArgValue(thisArgValue);
	return true;
}

bool
OperatorMgr::getEnumTypeMember(
	const Value& opValue,
	EnumType* enumType,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	FindModuleItemResult findResult = enumType->findItem(name);
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item)
	{
		err::setFormatStringError("'%s' is not a member of '%s'", name.sz(), enumType->getTypeString().sz());
		return false;
	}

	ASSERT(findResult.m_item->getItemKind() == ModuleItemKind_EnumConst);
	EnumConst* enumConst = (EnumConst*)findResult.m_item;
	Value memberValue(enumConst->getValue(), enumType);
	BinOpKind binOpKind = (enumType->getFlags() & EnumTypeFlag_BitFlag) ? BinOpKind_BwAnd : BinOpKind_Eq;

	return binaryOperator(
		binOpKind,
		opValue,
		memberValue,
		resultValue
		);
}

bool
OperatorMgr::getVariantMember(
	const Value& opValue,
	size_t index,
	Value* resultValue
	)
{
	Property* prop = m_module->m_functionMgr.getStdProperty(StdProp_VariantIndex);
	resultValue->setProperty(prop);

	Value variantValue;
	Value indexValue(index, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT));

	bool result = unaryOperator(UnOpKind_Addr, opValue, &variantValue);
	if (!result)
		return false;

	Closure* closure = resultValue->createClosure();
	closure->append(variantValue);
	closure->append(indexValue);
	return true;
}

bool
OperatorMgr::getVariantMember(
	const Value& opValue,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	Property* prop = m_module->m_functionMgr.getStdProperty(StdProp_VariantMember);
	resultValue->setProperty(prop);

	Value variantValue;
	Value nameValue = m_module->m_constMgr.saveLiteral(name);

	bool result = unaryOperator(UnOpKind_Addr, opValue, &variantValue);
	if (!result)
		return false;

	Closure* closure = resultValue->createClosure();
	closure->append(variantValue);
	closure->append(nameValue);
	return true;
}

bool
OperatorMgr::memberOperator(
	const Value& rawOpValue,
	size_t index,
	Value* resultValue
	)
{
	Value opValue;

	bool result = prepareOperand(
		rawOpValue,
		&opValue,
		OpFlag_KeepDataRef |
		OpFlag_KeepClassRef |
		OpFlag_EnsurePtrTargetLayout
		);

	if (!result)
		return false;

	Type* type = opValue.getType();
	TypeKind typeKind = type->getTypeKind();

	Field* field;

	switch (typeKind)
	{
	case TypeKind_DataPtr:
	case TypeKind_DataRef:
		type = ((DataPtrType*)type)->getTargetType();
		typeKind = type->getTypeKind();
		switch (typeKind)
		{
		case TypeKind_Array:
			return binaryOperator(
				BinOpKind_Idx,
				opValue,
				Value(index, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT)),
				resultValue
				);

		case TypeKind_Struct:
			field = ((StructType*)type)->getFieldByIndex(index);
			return field && getStructField(opValue, field, NULL, resultValue);

		case TypeKind_Union:
			field = ((UnionType*)type)->getFieldByIndex(index);
			return field && getUnionField(opValue, field, resultValue);

		case TypeKind_Variant:
			return getVariantMember(opValue, index, resultValue);

		default:
			err::setFormatStringError("indexed member operator cannot be applied to '%s'", type->getTypeString().sz());
			return false;
		}

	case TypeKind_ClassPtr:
	case TypeKind_ClassRef:
		type = ((ClassPtrType*)type)->getTargetType();
		field = ((ClassType*)type)->getFieldByIndex(index);
		return field && getClassField(opValue, field, NULL, resultValue);

	default:
		err::setFormatStringError("indexed member operator cannot be applied to '%s'", type->getTypeString().sz());
		return false;
	}
}

bool
OperatorMgr::getLibraryMember(
	DynamicLibNamespace* library,
	Closure* closure,
	const sl::StringRef& name,
	Value* resultValue
	)
{
	ASSERT(closure && closure->isMemberClosure());

	Value memberValue;
	bool result = getNamespaceMember(library, name, 0, &memberValue);
	if (!result)
		return false;

	if (memberValue.getValueKind() != ValueKind_Function)
	{
		*resultValue = memberValue;
		return true;
	}

	Function* function = memberValue.getFunction();
	size_t index = function->getLibraryTableIndex();
	const char* namePtr = function->getName(); // make sure name pointer stays valid (points to function, not token string)

	Value argValueArray[] =
	{
		closure->getThisArgValue(),
		Value(index, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT)),
		Value(&namePtr, m_module->m_typeMgr.getStdType(StdType_CharConstPtr)),
	};

	m_module->m_llvmIrBuilder.createBitCast(
		argValueArray[0],
		((ClassType*)m_module->m_typeMgr.getStdType(StdType_DynamicLib))->getClassPtrType(),
		&argValueArray[0]
		);

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	ASSERT(scope);

	Value ptrValue;

	Function* getterFunction = m_module->m_functionMgr.getStdFunction(StdFunc_TryLazyGetDynamicLibFunction);
	FunctionType* getterFunctionType = getterFunction->getType();

	m_module->m_llvmIrBuilder.createCall(
		getterFunction,
		getterFunctionType,
		argValueArray,
		countof(argValueArray),
		&ptrValue
		);

	m_module->m_controlFlowMgr.checkErrorCode(ptrValue, getterFunctionType->getReturnType());

	Type* resultType = function->getType()->getFunctionPtrType(FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe);
	m_module->m_llvmIrBuilder.createBitCast(ptrValue, resultType, resultValue);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
