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
#include "jnc_ct_ReactorClassType.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

ClassPtrType*
ClassType::getClassPtrType(
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return m_module->m_typeMgr.getClassPtrType(this, typeKind, ptrTypeKind, flags);
}

StructType*
ClassType::getVtableStructType() {
	if (m_vtableStructType)
		return m_vtableStructType;

	m_vtableStructType = m_module->m_typeMgr.createInternalStructType("!Vtable");
	m_vtableStructType->m_parentNamespace = this;
	return m_vtableStructType;
}

Field*
ClassType::createFieldImpl(
	const sl::StringRef& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::List<Token>* constructor,
	sl::List<Token>* initializer
) {
	Field* field = m_ifaceStructType->createField(name, type, bitCount, ptrTypeFlags, constructor, initializer);
	if (!field)
		return NULL;

	// re-parent

	field->m_parentNamespace = this;

	if (name.isEmpty()) {
		m_unnamedFieldArray.append(field);
	} else if (name[0] != '!') { // internal field
		bool result = addItem(field);
		if (!result)
			return NULL;
	}

	m_fieldArray.append(field);
	return field;
}

bool
ClassType::addMethod(Function* function) {
	StorageKind storageKind = function->getStorageKind();
	FunctionKind functionKind = function->getFunctionKind();
	uint_t functionKindFlags = getFunctionKindFlags(functionKind);
	uint_t thisArgTypeFlags = function->m_thisArgTypeFlags;

	function->m_parentNamespace = this;

	switch (storageKind) {
	case StorageKind_Static:
		if (thisArgTypeFlags) {
			err::setFormatStringError("static method cannot be '%s'", getPtrTypeFlagString(thisArgTypeFlags).sz());
			return false;
		}

		break;

	case StorageKind_Undefined:
		function->m_storageKind = StorageKind_Member;
		// and fall through

	case StorageKind_Member:
		function->convertToMemberMethod(this);
		break;

	case StorageKind_Override:
		m_overrideMethodArray.append(function);
		function->convertToMemberMethod(this);
		break;

	case StorageKind_Abstract:
	case StorageKind_Virtual:
		m_virtualMethodArray.append(function);
		function->convertToMemberMethod(this);
		break;

	default:
		err::setFormatStringError("invalid storage specifier '%s' for method member", getStorageKindString(storageKind));
		return false;
	}

	Property* indexerProperty;
	sl::Array<FunctionArg*> argArray;
	Function** targetFunction = NULL;
	OverloadableFunction* targetOverloadableFunction = NULL;
	size_t overloadIdx;

	switch (functionKind) {
	case FunctionKind_Internal:
	case FunctionKind_Reactor:
		return true;

	case FunctionKind_StaticConstructor:
		targetFunction = &m_staticConstructor;
		break;

	case FunctionKind_Constructor:
		targetOverloadableFunction = &m_constructor;
		break;

	case FunctionKind_Destructor:
		targetFunction = &m_destructor;
		break;

	case FunctionKind_Normal:
		overloadIdx = addFunction(function);
		if (overloadIdx == -1)
			return false;

		m_methodArray.append(function);
		return true;

	case FunctionKind_UnaryOperator:
		if (m_unaryOperatorTable.isEmpty())
			m_unaryOperatorTable.setCountZeroConstruct(UnOpKind__Count);

		targetOverloadableFunction = &m_unaryOperatorTable.rwi()[function->getUnOpKind()];
		break;

	case FunctionKind_BinaryOperator:
		if (m_binaryOperatorTable.isEmpty())
			m_binaryOperatorTable.setCountZeroConstruct(BinOpKind__Count);

		targetOverloadableFunction = &m_binaryOperatorTable.rwi()[function->getBinOpKind()];
		break;

	case FunctionKind_CallOperator:
		targetOverloadableFunction = &m_callOperator;
		break;

	case FunctionKind_Getter:
		argArray = function->getType()->getArgArray();
		if (argArray.getCount() < 2) {
			err::setError("indexer property getter should take at least one index argument");
			return false;
		}

		indexerProperty = getIndexerProperty(argArray[1]->getType());
		targetFunction = &indexerProperty->m_getter;
		break;

	case FunctionKind_Setter:
		argArray = function->getType()->getArgArray();
		if (argArray.getCount() < 3) {
			err::setError("indexer property setter should take at least one index argument");
			return false;
		}

		indexerProperty = getIndexerProperty(argArray[1]->getType());
		targetOverloadableFunction = &indexerProperty->m_setter;
		break;

	default:
		err::setFormatStringError(
			"invalid %s in '%s'",
			getFunctionKindString(functionKind),
			getTypeString().sz()
		);
		return false;
	}

	return addUnnamedMethod(function, targetFunction, targetOverloadableFunction);
}

bool
ClassType::addProperty(Property* prop) {
	if (prop->isNamed()) {
		bool result = addItem(prop);
		if (!result)
			return false;
	}

	prop->m_parentNamespace = this;

	StorageKind storageKind = prop->getStorageKind();
	switch (storageKind) {
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
		m_virtualPropertyArray.append(prop);
		prop->m_parentType = this;
		break;
	}

	m_propertyArray.append(prop);
	return true;
}

void
ClassType::prepareLlvmType() {
	m_llvmType = getClassStructType()->getLlvmType();
}

void
ClassType::prepareLlvmDiType() {
	m_llvmDiType = getClassStructType()->getLlvmDiType();
}

sl::StringRef
ClassType::createItemString(size_t index) {
	if (m_stdType != StdType_AbstractClass)
		return DerivableType::createItemString(index);

	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix:
		return "class";

	default:
		return DerivableType::createItemString(index);
	}
}

bool
ClassType::calcLayout() {
	bool result =
		ensureNamespaceReady() &&
		ensureAttributeValuesReady();

	if (!result)
		return false;

	// IfaceHdr (unless we can re-use the very first base class)

	if (m_baseTypeList.isEmpty()) {
		m_ifaceStructType->addBaseType(m_module->m_typeMgr.getStdType(StdType_IfaceHdr));
	} else {
		BaseTypeSlot* slot = *m_baseTypeList.getHead();

		result = slot->m_type->ensureLayout();
		if (!result)
			return false;

		if (slot->m_type->getTypeKind() != TypeKind_Class)
			m_ifaceStructType->addBaseType(m_module->m_typeMgr.getStdType(StdType_IfaceHdr));
	}

	// layout base types

	size_t baseTypeCount = m_baseTypeList.getCount();

	char buffer[256];
	sl::Array<BaseTypeSlot*> ifaceBaseTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	ifaceBaseTypeArray.setCount(baseTypeCount);
	sl::Array<BaseTypeSlot*>::Rwi baseTypeRwi = ifaceBaseTypeArray;

	sl::Iterator<BaseTypeSlot> slotIt = m_baseTypeList.getHead();
	for (size_t i = 0; slotIt; i++, slotIt++) {
		BaseTypeSlot* slot = *slotIt;
		result = slot->m_type->ensureLayout();
		if (!result)
			return false;

		if (!(slot->m_type->getTypeKindFlags() & TypeKindFlag_Derivable)) {
			err::setFormatStringError("'%s' cannot be a base type of a class", slot->m_type->getTypeString().sz());
			return false;
		}

		sl::StringHashTableIterator<BaseTypeSlot*> it = m_baseTypeMap.visit(slot->m_type->getSignature());
		if (it->m_value) {
			err::setFormatStringError(
				"'%s' is already a base type",
				slot->m_type->getTypeString().sz()
			);
			return false;
		}

		it->m_value = slot;

		DerivableType* type = slot->getType();
		result = type->ensureLayout();
		if (!result)
			return false;

		if (slot->m_type->getFlags() & TypeFlag_GcRoot) {
			m_gcRootBaseTypeArray.append(slot);
			m_flags |= TypeFlag_GcRoot;
		}

		if (slot->m_type->getConstructor())
			m_baseTypeConstructArray.append(slot);

		if (slot->m_type->getTypeKind() != TypeKind_Class) {
			baseTypeRwi[i] = m_ifaceStructType->addBaseType(slot->m_type);
			continue;
		}

		ClassType* baseClassType = (ClassType*)slot->m_type;
		if (baseClassType->m_flags & ClassTypeFlag_OpaqueNonCreatable) {
			err::setFormatStringError("cannot derive from non-creatable opaque '%s'", baseClassType->getTypeString().sz());
			return false;
		}

		baseTypeRwi[i] = m_ifaceStructType->addBaseType(baseClassType->getIfaceStructType());
		slot->m_vtableIndex = m_vtable.getCount();
		m_vtable.append(baseClassType->m_vtable);

		if (baseClassType->m_vtableStructType)
			getVtableStructType()->append(baseClassType->m_vtableStructType);

		m_classBaseTypeArray.append(slot);

		if (baseClassType->m_destructor)
			m_baseTypeDestructArray.append(slot);
	}

	// finalize iface layout

	result = m_ifaceStructType->ensureLayout();
	if (!result)
		return false;

	// scan members for gcroots and constructors

	size_t count = m_fieldArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Field* field = m_fieldArray[i];
		Type* type = field->getType();

		if (type->getFlags() & TypeFlag_GcRoot) {
			m_gcRootFieldArray.append(field);
			m_flags |= TypeFlag_GcRoot;
		}

		if (type->getTypeKind() == TypeKind_Class) {
			ClassType* classType = (ClassType*)type;
			if (classType->m_flags & (ClassTypeFlag_HasAbstractMethods | ClassTypeFlag_OpaqueNonCreatable)) {
				err::setFormatStringError("cannot instantiate '%s'", type->getTypeString().sz());
				return false;
			}

			if (classType->getClassTypeKind() == ClassTypeKind_Reactor)
				((ReactorClassType*)classType)->m_parentOffset = field->getOffset() + sizeof(Box); // reactor's box

			m_classFieldArray.append(field);
		}

		if (field->m_parentNamespace == this && // skip property fields
			(!field->m_initializer.isEmpty() || isConstructibleType(type))
		)
			m_fieldInitializeArray.append(field);
	}

	scanStaticVariables();
	scanPropertyCtorDtors();

	// update base type llvm indexes & offsets

	slotIt = m_baseTypeList.getHead();
	for (size_t i = 0; slotIt; i++, slotIt++) {
		BaseTypeSlot* slot = *slotIt;
		BaseTypeSlot* ifaceSlot = ifaceBaseTypeArray[i];

		slot->m_llvmIndex = ifaceSlot->m_llvmIndex;
		slot->m_offset = ifaceSlot->m_offset;
	}

	m_classStructType->ensureLayout();

	// layout virtual properties

	if (!m_virtualPropertyArray.isEmpty() || !m_virtualMethodArray.isEmpty())
		getVtableStructType(); // ensure Vtable struct

	count = m_virtualPropertyArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Property* prop = m_virtualPropertyArray[i];
		ASSERT(prop->m_storageKind == StorageKind_Abstract || prop->m_storageKind == StorageKind_Virtual);

		result = prop->ensureVtable();
		if (!result)
			return false;

		size_t vtableIndex = m_vtable.getCount();
		prop->m_parentClassVtableIndex = vtableIndex;
		m_vtable.append(prop->m_vtable);
		m_vtableStructType->append(prop->m_type->getVtableStructType());

		size_t accessorCount = prop->m_vtable.getCount();
		for (size_t j = 0; j < accessorCount; j++) {
			Function* accessor = prop->m_vtable[j];
			accessor->m_virtualOriginClassType = this;
			accessor->m_classVtableIndex = vtableIndex + j;
		}
	}

	// layout virtual methods

	count = m_virtualMethodArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Function* function = m_virtualMethodArray[i];
		ASSERT(function->m_storageKind == StorageKind_Abstract || function->m_storageKind == StorageKind_Virtual);

		result = addVirtualFunction(function);
		if (!result) {
			function->pushSrcPosError();
			return false;
		}
	}

	count = m_overrideMethodArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Function* function = m_overrideMethodArray[i];
		ASSERT(function->m_storageKind == StorageKind_Override);

		result = overrideVirtualFunction(function);
		if (!result) {
			function->pushSrcPosError();
			return false;
		}
	}

	if (m_vtableStructType) {
		result = m_vtableStructType->ensureLayout();
		if (!result)
			return false;
	}

	// create default constructors/destructors

	if (!m_staticConstructor &&
		(!m_staticVariableInitializeArray.isEmpty() ||
		!m_propertyStaticConstructArray.isEmpty())) {
		result = createDefaultMethod<DefaultStaticConstructor>() != NULL;
		if (!result)
			return false;
	}

	if (!m_constructor &&
		(m_staticConstructor ||
		!m_baseTypeConstructArray.isEmpty() ||
		!m_fieldInitializeArray.isEmpty() ||
		!m_propertyConstructArray.isEmpty())) {
		result = createDefaultMethod<DefaultConstructor>() != NULL;
		if (!result)
			return false;
	}

	if (!m_destructor &&
		(!m_baseTypeDestructArray.isEmpty() ||
		!m_propertyDestructArray.isEmpty())) {
		result = createDefaultMethod<DefaultDestructor>() != NULL;
		if (!result)
			return false;
	}

	m_size = m_classStructType->getSize();
	m_alignment = m_classStructType->getAlignment();
	return true;
}

bool
ClassType::addVirtualFunction(Function* function) {
	ASSERT(function->m_storageKind == StorageKind_Abstract || function->m_storageKind == StorageKind_Virtual);
	ASSERT(function->m_virtualOriginClassType == NULL); // not layed out yet

	bool result = function->getType()->ensureLayout();
	if (!result)
		return false;

	if (function->m_storageKind == StorageKind_Abstract)
		m_flags |= ClassTypeFlag_HasAbstractMethods;

	function->m_virtualOriginClassType = this;
	function->m_classVtableIndex = m_vtable.getCount();

	FunctionPtrType* pointerType = function->getType()->getFunctionPtrType(FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe);
	getVtableStructType()->createField(pointerType);
	m_vtable.append(function);
	return true;
}

bool
ClassType::overrideVirtualFunction(Function* function) {
	ASSERT(function->m_storageKind == StorageKind_Override);
	ASSERT(function->m_virtualOriginClassType == NULL); // not layed out yet

	bool result = function->getType()->ensureLayout();
	if (!result)
		return false;

	FunctionKind functionKind = function->getFunctionKind();
	FindModuleItemResult findResult = findDirectChildItemTraverse(
		function->m_name,
		NULL,
		TraverseFlag_NoExtensionNamespaces |
		TraverseFlag_NoParentNamespace |
		TraverseFlag_NoUsingNamespaces |
		TraverseFlag_NoThis
	);

	if (!findResult.m_result)
		return false;

	if (!findResult.m_item) {
		err::setFormatStringError("cannot override '%s': method not found", function->getItemName().sz());
		return false;
	}

	OverloadableFunction setter;
	FunctionOverload* overridenOverload = NULL;
	Function* overridenFunction = NULL;
	ModuleItem* member = findResult.m_item;
	ModuleItemKind itemKind = member->getItemKind();

	switch (itemKind) {
	case ModuleItemKind_FunctionOverload:
		overridenOverload = (FunctionOverload*)member;
		break;

	case ModuleItemKind_Function:
		overridenFunction = (Function*)member;
		break;

	case ModuleItemKind_Property:
		switch (functionKind) {
		case FunctionKind_Getter:
			overridenFunction = ((Property*)member)->getGetter();
			break;

		case FunctionKind_Setter:
			setter = ((Property*)member)->getSetter();
			if (!setter) {
				err::setFormatStringError("cannot override '%s': property has no setter", function->getItemName().sz());
				return false;
			}

			if (setter->getItemKind() == ModuleItemKind_Function)
				overridenFunction = setter.getFunction();
			else
				overridenOverload = setter.getFunctionOverload();

			break;

		default:
			err::setFormatStringError("cannot override '%s': function kind mismatch", function->getItemName().sz());
			return false;
		}

		break;

	default:
		err::setFormatStringError("cannot override '%s': not a method or property", function->getItemName().sz());
		return false;
	}

	if (overridenFunction) {
		ASSERT(!overridenOverload);
		result = overridenFunction->getType()->getShortType()->isEqual(function->getType()->getShortType());
	} else {
		ASSERT(overridenOverload);
		overridenFunction = overridenOverload->findShortOverload(function->getType()->getShortType());
		result = overridenFunction != NULL;
	}

	if (!result) {
		err::setFormatStringError("cannot override '%s': method signature mismatch", function->getItemName().sz());
		return false;
	}

	if (!overridenFunction->isVirtual()) {
		err::setFormatStringError("cannot override '%s': method is not virtual", function->getItemName().sz());
		return false;
	}

	AXL_TODO("virtual multipliers")

	ClassPtrType* thisArgType = (ClassPtrType*)overridenFunction->m_type->getThisArgType();
	ASSERT(thisArgType->getTypeKind() == TypeKind_ClassPtr);

	FunctionArg* origThisArg = function->m_type->m_argArray[0];
	FunctionArg* thisArg = m_module->m_typeMgr.getSimpleFunctionArg(StorageKind_This, thisArgType, origThisArg->getPtrTypeFlags());

	if (function->m_type->getFlags() & ModuleItemFlag_User)
		function->m_type->m_argArray.rwi()[0] = thisArg;
	else {
		sl::Array<FunctionArg*> argArray = function->m_type->m_argArray;
		argArray.ensureExclusive();
		argArray.rwi()[0] = thisArg;

		function->m_type = m_module->m_typeMgr.getFunctionType(
			function->m_type->getReturnType(),
			argArray,
			function->m_type->getFlags() & FunctionTypeFlag__All
		);
	}

	BaseTypeCoord coord;
	result = findBaseTypeTraverseImpl(overridenFunction->m_virtualOriginClassType, &coord, 0);
	ASSERT(result);

	function->m_thisArgType = thisArgType;
	function->m_thisArgDelta = -coord.m_offset;
	function->m_virtualOriginClassType = overridenFunction->m_virtualOriginClassType;
	function->m_classVtableIndex = overridenFunction->m_classVtableIndex;

	size_t vtableIndex = coord.m_vtableIndex + overridenFunction->m_classVtableIndex;
	ASSERT(vtableIndex < m_vtable.getCount());
	m_vtable.rwi()[vtableIndex] = function;
	return true;
}

bool
ClassType::ensureClassFieldsCreatable() {
	size_t count = m_classFieldArray.getCount();
	for (size_t i = 0; i < count; i++) {
		bool result = ((ClassType*)m_classFieldArray[i]->getType())->ensureCreatable();
		if (!result)
			return false;
	}

	return true;
}

bool
ClassType::prepareForOperatorNew() {
	ASSERT(m_flags & TypeFlag_LayoutReady);

	if (m_destructor)
		m_destructor->require();

	if (m_opaqueClassTypeInfo && m_opaqueClassTypeInfo->m_requireOpaqueItemsFunc)
		m_opaqueClassTypeInfo->m_requireOpaqueItemsFunc(m_module);

	size_t count = m_classBaseTypeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		bool result = ((ClassType*)m_classBaseTypeArray[i]->getType())->ensureClassFieldsCreatable();
		if (!result)
			return false;
	}

	ensureClassFieldsCreatable();

	if (!m_module->hasCodeGen() || !m_vtableStructType) {
		m_flags |= ClassTypeFlag_Creatable;
		return true;
	}

	count = m_vtable.getCount();

	char buffer[256];
	sl::Array<llvm::Constant*> llvmVtable(rc::BufKind_Stack, buffer, sizeof(buffer));
	llvmVtable.setCount(count);
	sl::Array<llvm::Constant*>::Rwi rwi = llvmVtable;

	for (size_t i = 0; i < count; i++) {
		Function* function = m_vtable[i];
		if (function->getStorageKind() == StorageKind_Abstract) {
			err::setFormatStringError("abstract class '%s'", getItemName().sz());
			return false;
		}

		function->require();
		rwi[i] = function->getLlvmFunction();
	}

	llvm::Constant* llvmVtableConst = llvm::ConstantStruct::get(
		(llvm::StructType*)m_vtableStructType->getLlvmType(),
		llvm::ArrayRef<llvm::Constant*>(llvmVtable, count)
	);

	m_vtableVariable = m_module->m_variableMgr.createSimpleStaticVariable(
		getLinkId() + "!m_vtable",
		m_vtableStructType,
		Value(llvmVtableConst, m_vtableStructType)
	);

	m_flags |= ClassTypeFlag_Creatable;
	return true;
}

void
ClassType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
) {
	Box* box = (Box*)p;
	IfaceHdr* iface = (IfaceHdr*)(box + 1);

	if (!iface->m_box) { // an unprimed static -- better make a shortcut exit
		ASSERT(!iface->m_vtable);
		return;
	}

	// we avoid adding invalidated objects to the mark queue in the first place;
	// however, we might attempt to mark an already-destructed static class object

	if (iface->m_box->m_flags & BoxFlag_Invalid)
		return;

	ASSERT(iface->m_box == box && box->m_type == this);
	markGcRootsImpl(iface, gcHeap);
}

void
ClassType::markGcRootsImpl(
	IfaceHdr* iface,
	rt::GcHeap* gcHeap
) {
	ClassType* boxType = (ClassType*)iface->m_box->m_type;
	ASSERT(boxType == this || boxType->findBaseTypeTraverse(this));

	char* p = (char*)iface;

	size_t count = m_gcRootBaseTypeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		BaseTypeSlot* slot = m_gcRootBaseTypeArray[i];
		Type* type = slot->getType();
		char* p2  = p + slot->getOffset();

		if (type->getTypeKind() == TypeKind_Class)
			((ClassType*)type)->markGcRootsImpl((IfaceHdr*)p2, gcHeap);
		else
			type->markGcRoots(p2, gcHeap);
	}

	count = m_gcRootFieldArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Field* field = m_gcRootFieldArray[i];
		Type* type = field->getType();
		char* p2 = p + field->getOffset();

		type->markGcRoots(p2, gcHeap);
	}

	if (m_opaqueClassTypeInfo && m_opaqueClassTypeInfo->m_markOpaqueGcRootsFunc)
		m_opaqueClassTypeInfo->m_markOpaqueGcRootsFunc(iface, gcHeap);
}

//..............................................................................

} // namespace ct
} // namespace jnc
