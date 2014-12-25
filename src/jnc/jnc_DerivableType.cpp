#include "pch.h"
#include "jnc_DerivableType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

BaseTypeSlot::BaseTypeSlot ()
{
	m_itemKind = ModuleItemKind_BaseTypeSlot;
	m_type = NULL;
	m_type_i = NULL;
	m_offset = 0;
	m_llvmIndex = -1;
	m_vtableIndex = -1;
}

//.............................................................................

BaseTypeCoord::BaseTypeCoord ():
	m_llvmIndexArray (ref::BufKind_Field, m_buffer, sizeof (m_buffer))
{
	m_type = NULL;
	m_offset = 0;
	m_vtableIndex = 0;
}

//.............................................................................

DerivableType::DerivableType ():
	NamedTypeBlock (this)
{
	m_defaultConstructor = NULL;
	m_callOperator = NULL;
	m_operatorVararg = NULL;
	m_operatorCdeclVararg = NULL;
	m_setAsType = NULL;
	m_setAsType_i = NULL;
}

FunctionType*
DerivableType::getMemberMethodType (
	FunctionType* shortType,
	uint_t thisArgTypeFlags
	)
{
	return m_module->m_typeMgr.getMemberMethodType (this, shortType, thisArgTypeFlags);
}

PropertyType*
DerivableType::getMemberPropertyType (PropertyType* shortType)
{
	return m_module->m_typeMgr.getMemberPropertyType (this, shortType);
}

ModuleItem*
DerivableType::findItemInExtensionNamespaces (const char* name)
{
	Namespace* nspace = m_module->m_namespaceMgr.getCurrentNamespace ();
	while (nspace)
	{
		ModuleItem* item = nspace->getUsingSet ()->findExtensionItem (this, name);
		if (item)
			return item;

		nspace = nspace->getParentNamespace ();
	}

	return NULL;
}

StructField*
DerivableType::getFieldByIndex(size_t index)
{
	if (!m_baseTypeList.isEmpty ())
	{
		err::setFormatStringError ("'%s' has base types, cannot use indexed member operator", getTypeString ().cc ());
		return NULL;
	}

	size_t count = m_memberFieldArray.getCount ();
	if (index >= count)
	{
		err::setFormatStringError ("index '%d' is out of bounds", index);
		return NULL;
	}

	return m_memberFieldArray [index];
}

Function*
DerivableType::getDefaultConstructor ()
{
	ASSERT (m_constructor);
	if (m_defaultConstructor)
		return m_defaultConstructor;

	Type* thisArgType = getThisArgType (PtrTypeFlag_Safe);

	// avoid allocations

	rtl::BoxListEntry <Value> thisArgValue;
	thisArgValue.m_value.setType (thisArgType);

	rtl::AuxList <rtl::BoxListEntry <Value> > argList;
	argList.insertTail (&thisArgValue);

	m_defaultConstructor = m_constructor->chooseOverload (argList);
	if (!m_defaultConstructor)
	{
		err::setFormatStringError ("'%s' has no default constructor", getTypeString ().cc ()); // thanks a lot gcc
		return NULL;
	}

	return m_defaultConstructor;
}

BaseTypeSlot*
DerivableType::getBaseTypeByIndex (size_t index)
{
	size_t count = m_baseTypeList.getCount ();
	if (index >= count)
	{
		err::setFormatStringError ("index '%d' is out of bounds", index);
		return NULL;
	}

	if (m_baseTypeArray.getCount () != count)
	{
		m_baseTypeArray.setCount (count);
		rtl::Iterator <BaseTypeSlot> slot = m_baseTypeList.getHead ();
		for (size_t i = 0; i < count; i++, slot++)
			m_baseTypeArray [i] = *slot;
	}

	return m_baseTypeArray [index];
}

BaseTypeSlot*
DerivableType::addBaseType (Type* type)
{
	rtl::StringHashTableMapIterator <BaseTypeSlot*> it = m_baseTypeMap.visit (type->getSignature ());
	if (it->m_value)
	{
		err::setFormatStringError (
			"'%s' is already a base type",
			type->getTypeString ().cc () // thanks a lot gcc
			);
		return NULL;
	}

	BaseTypeSlot* slot = AXL_MEM_NEW (BaseTypeSlot);
	slot->m_module = m_module;

	TypeKind typeKind = type->getTypeKind ();
	if (typeKind == TypeKind_NamedImport)
	{
		slot->m_type_i = (ImportType*) type;
		m_importBaseTypeArray.append (slot);
	}
	else if (
		(type->getTypeKindFlags () & TypeKindFlag_Derivable) &&
		(typeKind != TypeKind_Class || m_typeKind == TypeKind_Class))
	{
		slot->m_type = (DerivableType*) type;
	}
	else
	{
		err::setFormatStringError (
			"'%s' cannot be inherited from '%s'",
			getTypeString ().cc (),
			type->getTypeString ().cc ()
			);
		return NULL;
	}

	m_baseTypeList.insertTail (slot);
	it->m_value = slot;
	return slot;
}

bool
DerivableType::resolveImportTypes ()
{
	// base types

	size_t count = m_importBaseTypeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = m_importBaseTypeArray [i];
		ASSERT (slot->m_type_i);

		Type* type = slot->m_type_i->getActualType ();
		rtl::StringHashTableMapIterator <BaseTypeSlot*> it = m_baseTypeMap.visit (type->getSignature ());
		if (it->m_value)
		{
			err::setFormatStringError (
				"'%s' is already a base type",
				type->getTypeString ().cc () // thanks a lot gcc
				);
			return false;
		}

		if (!(type->getTypeKindFlags () & TypeKindFlag_Derivable) ||
			type->getTypeKind () == TypeKind_Class && m_typeKind != TypeKind_Class)
		{
			err::setFormatStringError (
				"'%s' cannot be inherited from '%s'",
				getTypeString ().cc (),
				type->getTypeString ().cc ()
				);
			return NULL;
		}

		slot->m_type = (DerivableType*) type;
		it->m_value = slot;
	}

	// fields

	count = m_importFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_importFieldArray [i];
		ASSERT (field->m_type_i);

		Type* type = field->m_type_i->getActualType ();
		if (field->m_type->getTypeKindFlags () & TypeKindFlag_Code)
		{
			err::setFormatStringError ("'%s': illegal type for a field", type->getTypeString ().cc ());
			return false;
		}

		field->m_type = type;
		
		if (field->m_bitCount)
		{
			ASSERT (field->m_bitFieldBaseType == field->m_type_i);
			field->m_bitFieldBaseType = type;
		}
	}

	// setas type

	if (m_setAsType_i)
		m_setAsType = m_setAsType_i->getActualType ();

	return true;
}

bool
DerivableType::addMethod (Function* function)
{
	StorageKind storageKind = function->getStorageKind ();
	FunctionKind functionKind = function->getFunctionKind ();
	uint_t functionKindFlags = getFunctionKindFlags (functionKind);
	uint_t thisArgTypeFlags = function->m_thisArgTypeFlags;

	function->m_parentNamespace = this;

	switch (storageKind)
	{
	case StorageKind_Static:
		if (thisArgTypeFlags)
		{
			err::setFormatStringError ("static method cannot be '%s'", getPtrTypeFlagString (thisArgTypeFlags).cc ());
			return false;
		}

		break;

	case StorageKind_Undefined:
		function->m_storageKind = StorageKind_Member;
		// and fall through

	case StorageKind_Member:
		function->convertToMemberMethod (this);
		break;

	default:
		err::setFormatStringError ("invalid storage specifier '%s' for method member", getStorageKindString (storageKind));
		return false;
	}

	Function** target = NULL;
	size_t overloadIdx;

	switch (functionKind)
	{
	case FunctionKind_PreConstructor:
		target = &m_preConstructor;
		break;

	case FunctionKind_Constructor:
		target = &m_constructor;
		break;

	case FunctionKind_StaticConstructor:
		target = &m_staticConstructor;
		m_module->m_functionMgr.addStaticConstructor (this);
		break;

	case FunctionKind_StaticDestructor:
		target = &m_staticDestructor;
		break;

	case FunctionKind_Named:
		overloadIdx = addFunction (function);
		if (overloadIdx == -1)
			return false;

		if (overloadIdx == 0)
			m_memberMethodArray.append (function);

		return true;

	case FunctionKind_UnaryOperator:
		if (m_unaryOperatorTable.isEmpty ())
			m_unaryOperatorTable.setCount (UnOpKind__Count);

		target = &m_unaryOperatorTable [function->getUnOpKind ()];
		break;

	case FunctionKind_BinaryOperator:
		if (m_binaryOperatorTable.isEmpty ())
			m_binaryOperatorTable.setCount (BinOpKind__Count);

		target = &m_binaryOperatorTable [function->getBinOpKind ()];
		break;

	case FunctionKind_CallOperator:
		target = &m_callOperator;
		break;

	case FunctionKind_OperatorVararg:
		target = &m_operatorVararg;
		break;

	case FunctionKind_OperatorCdeclVararg:
		target = &m_operatorCdeclVararg;
		break;

	default:
		err::setFormatStringError (
			"invalid %s in '%s'",
			getFunctionKindString (functionKind),
			getTypeString ().cc ()
			);
		return false;
	}

	function->m_tag.format ("%s.%s", m_tag.cc (), getFunctionKindString (functionKind));

	if (!*target)
	{
		*target = function;
	}
	else if (functionKindFlags & FunctionKindFlag_NoOverloads)
	{
		err::setFormatStringError (
			"'%s' already has '%s' method",
			getTypeString ().cc (),
			getFunctionKindString (functionKind)
			);
		return false;
	}
	else
	{
		bool result = (*target)->addOverload (function) != -1;
		if (!result)
			return false;
	}

	return true;
}

bool
DerivableType::addProperty (Property* prop)
{
	ASSERT (prop->isNamed ());
	bool result = addItem (prop);
	if (!result)
		return false;

	prop->m_parentNamespace = this;

	StorageKind storageKind = prop->getStorageKind ();
	switch (storageKind)
	{
	case StorageKind_Static:
		break;

	case StorageKind_Undefined:
		prop->m_storageKind = StorageKind_Member;
		//and fall through

	case StorageKind_Member:
		prop->m_parentType = this;
		break;

	default:
		err::setFormatStringError ("invalid storage specifier '%s' for method member", getStorageKindString (storageKind));
		return false;
	}

	m_memberPropertyArray.append (prop);
	return true;
}

bool
DerivableType::createDefaultMethod (
	FunctionKind functionKind,
	StorageKind storageKind
	)
{
	FunctionType* type = (FunctionType*) m_module->m_typeMgr.getStdType (StdType_SimpleFunction);
	Function* function = m_module->m_functionMgr.createFunction (functionKind, type);
	function->m_storageKind = storageKind;
	function->m_tag.format ("%s.%s", m_tag.cc (), getFunctionKindString (functionKind));

	bool result = addMethod (function);
	if (!result)
		return false;

	m_module->markForCompile (this);
	return true;
}

bool
DerivableType::compileDefaultStaticConstructor ()
{
	ASSERT (m_staticConstructor);

	m_module->m_functionMgr.internalPrologue (m_staticConstructor);

	bool result = initializeStaticFields ();
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

bool
DerivableType::compileDefaultPreConstructor ()
{
	ASSERT (m_preConstructor);

	bool result;

	Value thisValue;
	m_module->m_functionMgr.internalPrologue (m_preConstructor, &thisValue, 1);

	result = initializeMemberFields (thisValue);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

bool
DerivableType::compileDefaultConstructor ()
{
	ASSERT (m_constructor);

	bool result;

	Value thisValue;
	m_module->m_functionMgr.internalPrologue (m_constructor, &thisValue, 1);

	result =
		callBaseTypeConstructors (thisValue) &&
		callMemberFieldConstructors (thisValue) &&
		callMemberPropertyConstructors (thisValue);

	if (!result)
		return false;

	if (m_preConstructor)
	{
		result = m_module->m_operatorMgr.callOperator (m_preConstructor, thisValue);
		if (!result)
			return false;
	}

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

bool
DerivableType::compileDefaultDestructor ()
{
	ASSERT (m_destructor);

	bool result;

	Value argValue;
	m_module->m_functionMgr.internalPrologue (m_destructor, &argValue, 1);

	result =
		callMemberPropertyDestructors (argValue) &&
		callMemberFieldDestructors (argValue) &&
		callBaseTypeDestructors (argValue);

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

bool
DerivableType::callBaseTypeConstructors (const Value& thisValue)
{
	bool result;

	size_t count = m_baseTypeConstructArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = m_baseTypeConstructArray [i];
		if (slot->m_flags & ModuleItemFlag_Constructed)
		{
			slot->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		Function* constructor = slot->m_type->getDefaultConstructor ();
		if (!constructor)
			return false;

		result = m_module->m_operatorMgr.callOperator (constructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

bool
DerivableType::callBaseTypeDestructors (const Value& thisValue)
{
	bool result;

	size_t count = m_baseTypeDestructArray.getCount ();
	for (intptr_t i = count - 1; i >= 0; i--)
	{
		BaseTypeSlot* slot = m_baseTypeDestructArray [i];
		Function* destructor = slot->m_type->getDestructor ();
		ASSERT (destructor);

		result = m_module->m_operatorMgr.callOperator (destructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

bool
DerivableType::findBaseTypeTraverseImpl (
	Type* type,
	BaseTypeCoord* coord,
	size_t level
	)
{
	rtl::StringHashTableMapIterator <BaseTypeSlot*> it = m_baseTypeMap.find (type->getSignature ());
	if (it)
	{
		if (!coord)
			return true;

		BaseTypeSlot* slot = it->m_value;
		coord->m_type = slot->m_type;
		coord->m_offset = slot->m_offset;
		coord->m_vtableIndex = slot->m_vtableIndex;
		coord->m_llvmIndexArray.setCount (level + 1);
		coord->m_llvmIndexArray [level] = slot->m_llvmIndex;
		return true;
	}

	rtl::Iterator <BaseTypeSlot> slotIt = m_baseTypeList.getHead ();
	for (; slotIt; slotIt++)
	{
		BaseTypeSlot* slot = *slotIt;
		ASSERT (slot->m_type);

		bool result = slot->m_type->findBaseTypeTraverseImpl (type, coord, level + 1);
		if (result)
		{
			if (coord)
			{
				coord->m_offset += slot->m_offset;
				coord->m_vtableIndex += slot->m_vtableIndex;
				coord->m_llvmIndexArray [level] = slot->m_llvmIndex;
			}

			return true;
		}
	}

	return false;
}

ModuleItem*
DerivableType::findItemTraverseImpl (
	const char* name,
	MemberCoord* coord,
	uint_t flags,
	size_t level
	)
{
	ModuleItem* item;

	if (!(flags & TraverseKind_NoThis))
	{
		item = findItem (name);
		if (item)
		{
			if (coord)
			{
				coord->m_type = this;
				coord->m_llvmIndexArray.setCount (level);

				if (m_typeKind == TypeKind_Union)
				{
					UnionCoord unionCoord;
					unionCoord.m_type = (UnionType*) this;
					unionCoord.m_level = level;
					coord->m_unionCoordArray.insert (0, unionCoord);
				}
			}

			return item;
		}

		size_t count = m_unnamedFieldArray.getCount ();
		for	(size_t i = 0; i < count; i++)
		{
			StructField* field = m_unnamedFieldArray [i];
			if (field->getType ()->getTypeKindFlags () & TypeKindFlag_Derivable)
			{
				DerivableType* type = (DerivableType*) field->getType ();
				item = type->findItemTraverseImpl (name, coord, flags | TraverseKind_NoParentNamespace, level + 1);
				if (item)
				{
					if (coord)
					{
						coord->m_offset += field->m_offset;
						coord->m_llvmIndexArray [level] = field->m_llvmIndex;

						if (m_typeKind == TypeKind_Union)
						{
							UnionCoord unionCoord;
							unionCoord.m_type = (UnionType*) this;
							unionCoord.m_level = level;
							coord->m_unionCoordArray.insert (0, unionCoord);
						}
					}

					return item;
				}
			}
		}
	}

	if (!(flags & TraverseKind_NoExtensionNamespaces))
	{
		item = findItemInExtensionNamespaces (name);
		if (item)
			return item;
	}

	flags &= ~TraverseKind_NoThis;

	if (!(flags & TraverseKind_NoBaseType))
	{
		rtl::Iterator <BaseTypeSlot> slotIt = m_baseTypeList.getHead ();
		for (; slotIt; slotIt++)
		{
			BaseTypeSlot* slot = *slotIt;

			DerivableType* baseType = NULL;

			if (slot->m_type)
			{
				baseType = slot->m_type;
			}
			else if (slot->m_type_i && slot->m_type_i->isResolved ())
			{
				Type* actualType = slot->m_type_i->getActualType ();
				if (actualType->getTypeKindFlags () & TypeKindFlag_Derivable)
					baseType = (DerivableType*) actualType;
			}

			if (!baseType)
				return NULL;

			item = baseType->findItemTraverseImpl (name, coord, flags, level + 1);
			if (item)
			{
				if (coord)
				{
					coord->m_offset += slot->m_offset;
					coord->m_llvmIndexArray [level] = slot->m_llvmIndex;
					coord->m_vtableIndex += slot->m_vtableIndex;
				}

				return item;
			}
		}
	}

	if (!(flags & TraverseKind_NoParentNamespace) && m_parentNamespace)
	{
		item = m_parentNamespace->findItemTraverse (name, coord, flags);
		if (item)
			return item;
	}

	return NULL;
}

//.............................................................................

} // namespace jnc {
