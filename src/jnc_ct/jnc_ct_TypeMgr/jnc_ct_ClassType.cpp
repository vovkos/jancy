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
#include "jnc_ct_ClassType.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

ClassType::ClassType ()
{
	m_typeKind = TypeKind_Class;
	m_flags = TypeFlag_NoStack;
	m_ifaceStructType = NULL;
	m_classStructType = NULL;
	m_vtableStructType = NULL;
	m_markOpaqueGcRootsFunc = NULL;
	m_classPtrTypeTuple = NULL;
	m_vtableVariable = NULL;
}

ClassPtrType*
ClassType::getClassPtrType (
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	return m_module->m_typeMgr.getClassPtrType (this, typeKind, ptrTypeKind, flags);
}

StructType*
ClassType::getVTableStructType ()
{
	if (m_vtableStructType)
		return m_vtableStructType;

	m_vtableStructType = m_module->m_typeMgr.createUnnamedStructType ();
	m_vtableStructType->m_tag.format ("%s.VTable",m_tag.sz ());
	return m_vtableStructType;
}

StructField*
ClassType::createFieldImpl (
	const sl::StringRef& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::BoxList <Token>* constructor,
	sl::BoxList <Token>* initializer
	)
{
	if (m_flags & ModuleItemFlag_Sealed)
	{
		err::setFormatStringError ("'%s' is completed, cannot add fields to it", getTypeString ().sz ());
		return NULL;
	}

	StructField* field = m_ifaceStructType->createField (name, type, bitCount, ptrTypeFlags, constructor, initializer);
	if (!field)
		return NULL;

	// re-parent

	field->m_parentNamespace = this;

	if (name.isEmpty ())
	{
		m_unnamedFieldArray.append (field);
	}
	else if (name [0] != '!') // internal field
	{
		bool result = addItem (field);
		if (!result)
			return NULL;
	}

	if (!field->m_constructor.isEmpty () ||
		!field->m_initializer.isEmpty ())
	{
		m_initializedMemberFieldArray.append (field);
	}

	m_memberFieldArray.append (field);
	return field;
}

bool
ClassType::addMethod (Function* function)
{
	StorageKind storageKind = function->getStorageKind ();
	FunctionKind functionKind = function->getFunctionKind ();
	uint_t functionKindFlags = getFunctionKindFlags (functionKind);
	uint_t thisArgTypeFlags = function->m_thisArgTypeFlags;

	function->m_parentNamespace = this;

	if (storageKind == StorageKind_Undefined)
		storageKind = StorageKind_Member;

	switch (storageKind)
	{
	case StorageKind_Static:
		if (thisArgTypeFlags)
		{
			err::setFormatStringError ("static method cannot be '%s'", getPtrTypeFlagString (thisArgTypeFlags).sz ());
			return false;
		}

		break;

	case StorageKind_Member:
		function->convertToMemberMethod (this);
		break;

	case StorageKind_Override:
		m_overrideMethodArray.append (function);
		function->convertToMemberMethod (this);
		break;

	case StorageKind_Abstract:
	case StorageKind_Virtual:
		m_virtualMethodArray.append (function);
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
	case FunctionKind_Internal:
		return true;

	case FunctionKind_PreConstructor:
		target = &m_preconstructor;
		break;

	case FunctionKind_Constructor:
		target = &m_constructor;
		break;

	case FunctionKind_Destructor:
		target = &m_destructor;
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
			m_unaryOperatorTable.setCountZeroConstruct (UnOpKind__Count);

		target = &m_unaryOperatorTable [function->getUnOpKind ()];
		break;

	case FunctionKind_BinaryOperator:
		if (m_binaryOperatorTable.isEmpty ())
			m_binaryOperatorTable.setCountZeroConstruct (BinOpKind__Count);

		target = &m_binaryOperatorTable [function->getBinOpKind ()];
		break;

	case FunctionKind_CallOperator:
		target = &m_callOperator;
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

	if (function->m_tag.isEmpty ())
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
ClassType::addProperty (Property* prop)
{
	if (prop->isNamed ())
	{
		bool result = addItem (prop);
		if (!result)
			return false;
	}

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

	case StorageKind_Abstract:
	case StorageKind_Virtual:
	case StorageKind_Override:
		m_virtualPropertyArray.append (prop);
		prop->m_parentType = this;
		break;
	}

	m_memberPropertyArray.append (prop);
	return true;
}

bool
ClassType::calcLayout ()
{
	bool result;

	// layout base types

	if (m_baseTypeList.isEmpty () ||
		m_baseTypeList.getHead ()->getType ()->getTypeKind () != TypeKind_Class)
	{
		m_ifaceStructType->addBaseType (m_module->m_typeMgr.getStdType (StdType_IfaceHdr));
	}

	size_t baseTypeCount = m_baseTypeList.getCount ();

	char buffer [256];
	sl::Array <BaseTypeSlot*> ifaceBaseTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	ifaceBaseTypeArray.setCount (baseTypeCount);

	sl::Iterator <BaseTypeSlot> slotIt = m_baseTypeList.getHead ();
	for (size_t i = 0; slotIt; i++, slotIt++)
	{
		BaseTypeSlot* slot = *slotIt;
		if (!(slot->m_type->getTypeKindFlags () & TypeKindFlag_Derivable) ||
			(slot->m_type->getFlags () & TypeFlag_Dynamic))
		{
			err::setFormatStringError ("'%s' cannot be a base type of a class", slot->m_type->getTypeString ().sz ());
			return false;
		}

		sl::StringHashTableIterator <BaseTypeSlot*> it = m_baseTypeMap.visit (slot->m_type->getSignature ());
		if (it->m_value)
		{
			err::setFormatStringError (
				"'%s' is already a base type",
				slot->m_type->getTypeString ().sz ()
				);
			return false;
		}

		it->m_value = slot;

		DerivableType* type = slot->getType ();
		result = type->ensureLayout ();
		if (!result)
			return false;

		if (slot->m_type->getFlags () & TypeFlag_GcRoot)
		{
			m_gcRootBaseTypeArray.append (slot);
			m_flags |= TypeFlag_GcRoot;
		}

		if (slot->m_type->getConstructor ())
			m_baseTypeConstructArray.append (slot);

		if (slot->m_type->getTypeKind () != TypeKind_Class)
		{
			ifaceBaseTypeArray [i] = m_ifaceStructType->addBaseType (slot->m_type);
			continue;
		}

		ClassType* baseClassType = (ClassType*) slot->m_type;
		if (baseClassType->m_flags & ClassTypeFlag_OpaqueNonCreatable)
		{
			err::setFormatStringError ("cannot derive from non-creatable opaque '%s'", baseClassType->getTypeString ().sz ());
			return false;
		}

		ifaceBaseTypeArray [i] = m_ifaceStructType->addBaseType (baseClassType->getIfaceStructType ());
		slot->m_vtableIndex = m_vtable.getCount ();
		m_vtable.append (baseClassType->m_vtable);

		if (baseClassType->m_vtableStructType)
			getVTableStructType ()->append (baseClassType->m_vtableStructType);

		m_baseTypePrimeArray.append (slot);

		if (baseClassType->m_destructor)
			m_baseTypeDestructArray.append (slot);
	}

	// finalize iface layout

	result = m_ifaceStructType->ensureLayout ();
	if (!result)
		return false;

	// scan members for gcroots, constructors & destructors

	bool hasClassFieldDestructors = false;

	size_t count = m_memberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_memberFieldArray [i];
		Type* type = field->getType ();

		if (type->getFlags () & TypeFlag_GcRoot)
		{
			m_gcRootMemberFieldArray.append (field);
			m_flags |= TypeFlag_GcRoot;
		}

		if ((type->getTypeKindFlags () & TypeKindFlag_Derivable) && ((DerivableType*) type)->getConstructor ())
			m_memberFieldConstructArray.append (field);

		if (type->getTypeKind () == TypeKind_Class)
		{
			ClassType* classType = (ClassType*) type;
			if (classType->getDestructor ())
				hasClassFieldDestructors = true;

			if (classType->m_flags & (ClassTypeFlag_HasAbstractMethods | ClassTypeFlag_OpaqueNonCreatable))
			{
				err::setFormatStringError ("cannot instantiate '%s'", type->getTypeString ().sz ());
				return false;
			}

			if (classType->getClassTypeKind () == ClassTypeKind_Reactor)
				((ReactorClassType*) classType)->m_parentOffset = field->getOffset () + sizeof (Box); // reactor's box

			m_classMemberFieldArray.append (field);
		}
	}

	count = m_memberPropertyArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Property* prop = m_memberPropertyArray [i];
		result = prop->ensureLayout ();
		if (!result)
			return false;

		if (prop->getConstructor ())
			m_memberPropertyConstructArray.append (prop);

		if (prop->getDestructor ())
			m_memberPropertyDestructArray.append (prop);
	}

	// update base type llvm indexes & offsets

	slotIt = m_baseTypeList.getHead ();
	for (size_t i = 0; slotIt; i++, slotIt++)
	{
		BaseTypeSlot* slot = *slotIt;
		BaseTypeSlot* ifaceSlot = ifaceBaseTypeArray [i];

		slot->m_llvmIndex = ifaceSlot->m_llvmIndex;
		slot->m_offset = ifaceSlot->m_offset;
	}

	m_classStructType->ensureLayout ();

	// layout virtual properties

	if (!m_virtualPropertyArray.isEmpty () || !m_virtualMethodArray.isEmpty ())
		getVTableStructType (); // ensure VTable struct

	count = m_virtualPropertyArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Property* prop = m_virtualPropertyArray [i];
		ASSERT (prop->m_storageKind == StorageKind_Abstract || prop->m_storageKind == StorageKind_Virtual);

		size_t VTableIndex = m_vtable.getCount ();

		prop->m_parentClassVTableIndex = VTableIndex;
		m_vtable.append (prop->m_vtable);
		m_vtableStructType->append (prop->m_type->getVTableStructType ());

		size_t accessorCount = prop->m_vtable.getCount ();
		for (size_t j = 0; j < accessorCount; j++)
		{
			Function* accessor = prop->m_vtable [j];
			accessor->m_virtualOriginClassType = this;
			accessor->m_classVTableIndex = VTableIndex + j;
		}
	}

	// layout virtual methods

	count = m_virtualMethodArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Function* function = m_virtualMethodArray [i];
		ASSERT (function->m_storageKind == StorageKind_Abstract || function->m_storageKind == StorageKind_Virtual);

		result = function->getType ()->ensureLayout ();
		if (!result)
			return false;

		addVirtualFunction (function);
	}

	count = m_overrideMethodArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Function* function = m_overrideMethodArray [i];
		ASSERT (function->m_storageKind == StorageKind_Override);

		result = function->getType ()->ensureLayout ();
		if (!result)
			return false;

		result = overrideVirtualFunction (function);
		if (!result)
			return false;
	}

	if (m_vtableStructType)
	{
		result = m_vtableStructType->ensureLayout ();
		if (!result)
			return false;

		createVTableVariable ();
	}

	if (!m_staticConstructor && !m_initializedStaticFieldArray.isEmpty ())
	{
		result = createDefaultMethod (FunctionKind_StaticConstructor, StorageKind_Static) != NULL;
		if (!result)
			return false;
	}

	if (!m_constructor &&
		(m_preconstructor ||
		!m_baseTypeConstructArray.isEmpty () ||
		!m_memberFieldConstructArray.isEmpty () ||
		!m_initializedMemberFieldArray.isEmpty () ||
		!m_memberPropertyConstructArray.isEmpty ()))
	{
		result = createDefaultMethod (FunctionKind_Constructor) != NULL;
		if (!result)
			return false;
	}

	if (!m_destructor &&
		(!m_baseTypeDestructArray.isEmpty () ||
		!m_memberPropertyDestructArray.isEmpty () ||
		hasClassFieldDestructors))
	{
		result = createDefaultMethod (FunctionKind_Destructor) != NULL;
		if (!result)
			return false;
	}

	m_size = m_classStructType->getSize ();
	m_alignment = m_classStructType->getAlignment ();
	return true;
}

void
ClassType::addVirtualFunction (Function* function)
{
	ASSERT (function->m_storageKind == StorageKind_Abstract || function->m_storageKind == StorageKind_Virtual);
	ASSERT (function->m_virtualOriginClassType == NULL); // not layed out yet

	function->m_virtualOriginClassType = this;
	function->m_classVTableIndex = m_vtable.getCount ();

	FunctionPtrType* pointerType = function->getType ()->getFunctionPtrType (FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe);
	getVTableStructType ()->createField (pointerType);
	m_vtable.append (function);
}

bool
ClassType::overrideVirtualFunction (Function* function)
{
	ASSERT (function->m_storageKind == StorageKind_Override);
	ASSERT (function->m_virtualOriginClassType == NULL); // not layed out yet

	FunctionKind functionKind = function->getFunctionKind ();

	MemberCoord coord;
	ModuleItem* member = findItemTraverse (
		function->m_declaratorName,
		&coord,
		TraverseKind_NoExtensionNamespaces |
		TraverseKind_NoParentNamespace |
		TraverseKind_NoUsingNamespaces |
		TraverseKind_NoThis
		);

	if (!member)
	{
		err::setFormatStringError ("cannot override '%s': method not found", function->m_tag.sz ());
		return false;
	}

	Function* overridenFunction;

	ModuleItemKind itemKind = member->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Function:
		if (functionKind != FunctionKind_Named)
		{
			err::setFormatStringError (
				"cannot override '%s': function kind mismatch",
				function->m_tag.sz ()
				);
			return false;
		}

		overridenFunction = (Function*) member;
		break;

	case ModuleItemKind_Property:
		switch (functionKind)
		{
		case FunctionKind_Getter:
			overridenFunction = ((Property*) member)->getGetter ();
			break;

		case FunctionKind_Setter:
			overridenFunction = ((Property*) member)->getSetter ();
			if (!overridenFunction)
			{
				err::setFormatStringError ("cannot override '%s': property has no setter", function->m_tag.sz ());
				return false;
			}

			break;

		default:
			err::setFormatStringError ("cannot override '%s': function kind mismatch", function->m_tag.sz ());
			return false;
		}

		break;

	default:
		err::setFormatStringError ("cannot override '%s': not a method or property", function->m_tag.sz ());
		return false;
	}

	overridenFunction = overridenFunction->findShortOverload (function->getType ()->getShortType ());
	if (!overridenFunction)
	{
		err::setFormatStringError ("cannot override '%s': method signature mismatch", function->m_tag.sz ());
		return false;
	}

	if (!overridenFunction->isVirtual ())
	{
		err::setFormatStringError ("cannot override '%s': method is not virtual", function->m_tag.sz ());
		return false;
	}

	AXL_TODO ("virtual multipliers")

	ClassPtrType* thisArgType = (ClassPtrType*) overridenFunction->m_type->getThisArgType ();
	ASSERT (thisArgType->getTypeKind () == TypeKind_ClassPtr);

	FunctionArg* origThisArg = function->m_type->m_argArray [0];
	FunctionArg* thisArg = m_module->m_typeMgr.getSimpleFunctionArg (StorageKind_This, thisArgType, origThisArg->getPtrTypeFlags ());

	if (function->m_type->getFlags () & ModuleItemFlag_User)
	{
		function->m_type->m_argArray [0] = thisArg;
	}
	else
	{
		sl::Array <FunctionArg*> argArray = function->m_type->m_argArray;
		argArray.ensureExclusive ();
		argArray [0] = thisArg;

		function->m_type = m_module->m_typeMgr.getFunctionType (
			function->m_type->getReturnType (),
			argArray,
			function->m_type->getFlags ()
			);
	}

	function->m_thisArgType = thisArgType;
	function->m_thisArgDelta = overridenFunction->m_thisArgDelta - coord.m_offset;
	function->m_virtualOriginClassType = overridenFunction->m_virtualOriginClassType;
	function->m_classVTableIndex = overridenFunction->m_classVTableIndex;

	size_t VTableIndex = coord.m_vtableIndex + overridenFunction->m_classVTableIndex;
	ASSERT (VTableIndex < m_vtable.getCount ());
	m_vtable [VTableIndex] = function;
	return true;
}

void
ClassType::createVTableVariable ()
{
	ASSERT (m_vtableStructType);

	char buffer [256];
	sl::Array <llvm::Constant*> llvmVTable (ref::BufKind_Stack, buffer, sizeof (buffer));

	size_t count = m_vtable.getCount ();
	llvmVTable.setCount (count);

	for (size_t i = 0; i < count; i++)
	{
		Function* function = m_vtable [i];
		if (function->getStorageKind () == StorageKind_Abstract)
		{
			function = function->getType ()->getAbstractFunction ();
			m_flags |= ClassTypeFlag_HasAbstractMethods;
		}

		llvmVTable [i] = function->getLlvmFunction ();
	}

	llvm::Constant* llvmVTableConst = llvm::ConstantStruct::get (
		(llvm::StructType*) m_vtableStructType->getLlvmType (),
		llvm::ArrayRef <llvm::Constant*> (llvmVTable, count)
		);

	m_vtableVariable = m_module->m_variableMgr.createSimpleStaticVariable (
		"m_vtable",
		m_tag + ".m_vtable",
		m_vtableStructType,
		Value (llvmVTableConst, m_vtableStructType)
		);
}

bool
ClassType::compile ()
{
	bool result;

	if (m_staticConstructor && !(m_staticConstructor->getFlags () & ModuleItemFlag_User))
	{
		result = compileDefaultStaticConstructor ();
		if (!result)
			return false;
	}

	if (m_constructor && !(m_constructor->getFlags () & ModuleItemFlag_User))
	{
		result = compileDefaultConstructor ();
		if (!result)
			return false;
	}

	if (m_destructor && !(m_destructor->getFlags () & ModuleItemFlag_User))
	{
		result = compileDefaultDestructor ();
		if (!result)
			return false;
	}

	return true;
}

void
ClassType::markGcRoots (
	const void* p,
	rt::GcHeap* gcHeap
	)
{
	Box* box = (Box*) p;
	IfaceHdr* iface = (IfaceHdr*) (box + 1);

	ASSERT (iface->m_box == box && box->m_type == this);

	markGcRootsImpl (iface, gcHeap);
}

void
ClassType::markGcRootsImpl (
	IfaceHdr* iface,
	rt::GcHeap* gcHeap
	)
{
	char* p = (char*) iface;

	size_t count = m_gcRootBaseTypeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = m_gcRootBaseTypeArray [i];
		Type* type = slot->getType ();
		char* p2  = p + slot->getOffset ();

		if (type->getTypeKind () == TypeKind_Class)
			((ClassType*) type)->markGcRootsImpl ((IfaceHdr*) p2, gcHeap);
		else
			type->markGcRoots (p2, gcHeap);
	}

	count = m_gcRootMemberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_gcRootMemberFieldArray [i];
		Type* type = field->getType ();
		char* p2 = p + field->getOffset ();

		type->markGcRoots (p2, gcHeap);
	}

	if (m_markOpaqueGcRootsFunc)
	{
		ClassType* boxType = (ClassType*) iface->m_box->m_type;
		ASSERT (isOpaqueClassType (boxType) && (boxType == this || boxType->findBaseTypeTraverse (this)));

		m_markOpaqueGcRootsFunc (iface, gcHeap);
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
