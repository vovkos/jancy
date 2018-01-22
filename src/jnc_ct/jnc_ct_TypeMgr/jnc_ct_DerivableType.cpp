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
#include "jnc_ct_DerivableType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

BaseTypeSlot::BaseTypeSlot ()
{
	m_itemKind = ModuleItemKind_BaseTypeSlot;
	m_type = NULL;
	m_offset = 0;
	m_llvmIndex = -1;
	m_vtableIndex = -1;
}

//..............................................................................

BaseTypeCoord::BaseTypeCoord ():
	m_llvmIndexArray (ref::BufKind_Field, m_buffer, sizeof (m_buffer))
{
	m_type = NULL;
	m_offset = 0;
	m_vtableIndex = 0;
}

//..............................................................................

DerivableType::DerivableType ():
	NamedTypeBlock (this)
{
	m_defaultConstructor = NULL;
	m_callOperator = NULL;
	m_operatorVararg = NULL;
	m_operatorCdeclVararg = NULL;
	m_setAsType = NULL;
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
DerivableType::findItemInExtensionNamespaces (const sl::StringRef& name)
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
		err::setFormatStringError ("'%s' has base types, cannot use indexed member operator", getTypeString ().sz ());
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

	sl::BoxListEntry <Value> thisArgValue;
	thisArgValue.m_value.setType (thisArgType);

	sl::AuxList <sl::BoxListEntry <Value> > argList;
	argList.insertTail (&thisArgValue);

	m_defaultConstructor = m_constructor->chooseOverload (argList);
	if (!m_defaultConstructor)
	{
		err::setFormatStringError ("'%s' has no default constructor", getTypeString ().sz ());
		return NULL;
	}

	return m_defaultConstructor;
}

Property*
DerivableType::getIndexerProperty (Type* argType)
{
	ASSERT (!(m_flags & ModuleItemFlag_LayoutReady));

	sl::StringHashTableIterator <Property*> it = m_indexerPropertyMap.visit (argType->getSignature ());
	if (it->m_value)
		return it->m_value;

	Property* prop = m_module->m_functionMgr.createProperty (PropertyKind_Internal, m_tag + ".m_indexer");
	prop->m_storageKind = StorageKind_Member;
	it->m_value = prop;
	return prop;
}

Property*
DerivableType::chooseIndexerProperty (const Value& opValue)
{
	CastKind bestCastKind = CastKind_None;
	Property* bestProperty = NULL;
	bool isAmbiguous = false;

	sl::StringHashTableIterator <Property*> it = m_indexerPropertyMap.getHead ();
	for (; it; it++)
	{
		Property* prop = it->m_value;

		FunctionArg* indexArg = prop->m_getter->getType ()->getArgArray () [1];
		CastKind castKind = m_module->m_operatorMgr.getCastKind (opValue, indexArg->getType ());
		if (!castKind)
			continue;

		if (castKind == bestCastKind)
			isAmbiguous = true;

		if (castKind > bestCastKind)
		{
			bestProperty = prop;
			bestCastKind = castKind;
			isAmbiguous = false;
		}
	}

	if (!bestProperty)
	{
		err::setFormatStringError ("none of the %d indexer properties accept the specified index argument", m_indexerPropertyMap.getCount ());
		return NULL;
	}

	if (isAmbiguous)
	{
		err::setFormatStringError ("ambiguous call to overloaded function");
		return NULL;
	}

	return bestProperty;
}

BaseTypeSlot*
DerivableType::getBaseTypeByIndex (size_t index)
{
	size_t count = m_baseTypeArray.getCount ();
	if (index >= count)
	{
		err::setFormatStringError ("index '%d' is out of bounds", index);
		return NULL;
	}

	return m_baseTypeArray [index];
}

BaseTypeSlot*
DerivableType::addBaseType (Type* type)
{
	BaseTypeSlot* slot = AXL_MEM_NEW (BaseTypeSlot);
	slot->m_module = m_module;
	slot->m_type = (DerivableType*) type;

	if (type->getTypeKindFlags () & TypeKindFlag_Import)
		((ImportType*) type)->addFixup ((Type**) &slot->m_type);

	m_baseTypeList.insertTail (slot);
	m_baseTypeArray.append (slot);
	return slot;
}

size_t
DerivableType::findBaseTypeOffset (Type* type)
{
	jnc::ct::BaseTypeCoord coord;
	bool result = findBaseTypeTraverse (type, &coord);
	return result ? coord.m_offset : -1;
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
			err::setFormatStringError ("static method cannot be '%s'", getPtrTypeFlagString (thisArgTypeFlags).sz ());
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

	Property* indexerProperty;
	sl::Array <FunctionArg*> argArray;
	Function** target = NULL;
	size_t overloadIdx;

	switch (functionKind)
	{
	case FunctionKind_PreConstructor:
		target = &m_preconstructor;
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

	case FunctionKind_Getter:
		argArray = function->getType ()->getArgArray ();
		if (argArray.getCount () < 2)
		{
			err::setFormatStringError ("indexer property getter should take at least one index argument");
			return false;
		}

		indexerProperty = getIndexerProperty (argArray [1]->getType ());
		target = &indexerProperty->m_getter;
		break;

	case FunctionKind_Setter:
		argArray = function->getType ()->getArgArray ();
		if (argArray.getCount () < 3)
		{
			err::setFormatStringError ("indexer property setter should take at least one index argument");
			return false;
		}

		indexerProperty = getIndexerProperty (argArray [1]->getType ());
		target = &indexerProperty->m_setter;
		break;

	default:
		err::setFormatStringError (
			"invalid %s in '%s'",
			getFunctionKindString (functionKind),
			getTypeString ().sz ()
			);
		return false;
	}

	function->m_tag.format ("%s.%s", m_tag.sz (), getFunctionKindString (functionKind));

	if (!*target)
	{
		*target = function;
	}
	else if (functionKindFlags & FunctionKindFlag_NoOverloads)
	{
		err::setFormatStringError (
			"'%s' already has '%s' method",
			getTypeString ().sz (),
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
	function->m_tag.format ("%s.%s", m_tag.sz (), getFunctionKindString (functionKind));

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
DerivableType::compileDefaultConstructor ()
{
	ASSERT (m_constructor);

	bool result;

	Value thisValue;
	m_module->m_functionMgr.internalPrologue (m_constructor, &thisValue, 1);

	result =
		callBaseTypeConstructors (thisValue) &&
		callMemberFieldConstructors (thisValue) &&
		initializeMemberFields (thisValue) &&
		callMemberPropertyConstructors (thisValue);

	if (!result)
		return false;

	if (m_preconstructor)
	{
		result = m_module->m_operatorMgr.callOperator (m_preconstructor, thisValue);
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
	sl::StringHashTableIterator <BaseTypeSlot*> it = m_baseTypeMap.find (type->getSignature ());
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

	sl::Iterator <BaseTypeSlot> slotIt = m_baseTypeList.getHead ();
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
	const sl::StringRef& name,
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

		uint_t modFlags = flags | TraverseKind_NoParentNamespace;
		size_t nextLevel = level + 1;

		size_t count = m_unnamedFieldArray.getCount ();
		for	(size_t i = 0; i < count; i++)
		{
			StructField* field = m_unnamedFieldArray [i];
			if (field->getType ()->getTypeKindFlags () & TypeKindFlag_Derivable)
			{
				DerivableType* type = (DerivableType*) field->getType ();
				item = type->findItemTraverseImpl (name, coord, modFlags, nextLevel);
				if (item)
				{
					if (coord && coord->m_type)
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

	if (!(flags & TraverseKind_NoBaseType))
	{
		uint_t modFlags = (flags & ~TraverseKind_NoThis) | TraverseKind_NoParentNamespace;
		size_t nextLevel = level + 1;

		sl::Iterator <BaseTypeSlot> slotIt = m_baseTypeList.getHead ();
		for (; slotIt; slotIt++)
		{
			BaseTypeSlot* slot = *slotIt;

			DerivableType* baseType = slot->m_type;
			if (baseType->getTypeKindFlags () & TypeKindFlag_Import)
			{
				ASSERT (false);
				return NULL;
			}

			item = baseType->findItemTraverseImpl (name, coord, modFlags, nextLevel);
			if (item)
			{
				if (coord && coord->m_type)
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

bool
DerivableType::generateDocumentation (
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	bool result;

	DoxyBlock* doxyBlock = getDoxyBlock ();

	const char* kind =
		m_typeKind == TypeKind_Struct ? "struct" :
		m_typeKind == TypeKind_Union ? "union" : "class";

	indexXml->appendFormat (
		"<compound kind='%s' refid='%s'><name>%s</name></compound>\n",
		kind,
		doxyBlock->getRefId ().sz (),
		getQualifiedName ().sz ()
		);

	sl::String constructorXml;
	sl::String destructorXml;
	if (m_constructor)
	{
		result = m_constructor->generateDocumentation (outputDir, &constructorXml, indexXml);
		if (!result)
			return false;
	}

	if (m_destructor)
	{
		result = m_destructor->generateDocumentation (outputDir, &destructorXml, indexXml);
		if (!result)
			return false;
	}

	sl::String memberXml;
	result = Namespace::generateMemberDocumentation (outputDir, &memberXml, indexXml, true);
	if (!result)
		return false;

	itemXml->format (
		"<compounddef kind='%s' id='%s' language='Jancy'>\n"
		"<compoundname>%s</compoundname>\n\n",
		kind,
		doxyBlock->getRefId ().sz (),
		m_name.sz ()
		);

	sl::Iterator <BaseTypeSlot> it = m_baseTypeList.getHead ();
	for (; it; it++)
	{
		DerivableType* baseType = it->getType ();
		sl::String refId = baseType->getDoxyBlock ()->getRefId ();
		Unit* unit = baseType->getParentUnit ();
		ExtensionLib* lib = unit ? unit->getLib () : NULL;
		if (lib)
			itemXml->appendFormat ("<basecompoundref importid='%s/%s'>", lib->m_guid->getGuidString ().sz (), refId.sz ());
		else
			itemXml->appendFormat ("<basecompoundref refid='%s'>", refId.sz ());

		itemXml->appendFormat ("%s</basecompoundref>\n", baseType->getQualifiedName ().sz ());
	}

	if (!constructorXml.isEmpty () || !destructorXml.isEmpty ())
	{
		itemXml->append ("<sectiondef>\n");
		itemXml->append (constructorXml);
		itemXml->append (destructorXml);
		itemXml->append ("</sectiondef>\n\n");
	}

	itemXml->append (memberXml);

	sl::String footnoteXml = doxyBlock->getFootnoteString ();
	if (!footnoteXml.isEmpty ())
	{
		itemXml->append ("<sectiondef>\n");
		itemXml->append (footnoteXml);
		itemXml->append ("</sectiondef>\n");
	}

	itemXml->append (doxyBlock->getImportString ());
	itemXml->append (doxyBlock->getDescriptionString ());
	itemXml->append (getDoxyLocationString ());
	itemXml->append ("</compounddef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
