#include "pch.h"
#include "jnc_ClassType.h"
#include "jnc_Module.h"
#include "jnc_Parser.llk.h"

namespace jnc {

//.............................................................................

ClassType::ClassType ()
{
	m_typeKind = TypeKind_Class;
	m_ifaceStructType = NULL;
	m_classStructType = NULL;
	m_extensionNamespace = NULL;
	m_primer = NULL;
	m_destructor = NULL;
	m_operatorNew = NULL;
	m_pVTableStructType = NULL;
	m_classPtrTypeTuple = NULL;
}

ClassPtrType*
ClassType::getClassPtrType (
	Namespace* anchorNamespace,
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	return m_module->m_typeMgr.getClassPtrType (anchorNamespace, this, typeKind, ptrTypeKind, flags);
}

StructField*
ClassType::getFieldByIndex (size_t index)
{
	if (!m_baseTypeList.isEmpty ())
	{
		err::setFormatStringError ("'%s' has base types, cannot use indexed member operator", getTypeString ().cc ());
		return NULL;
	}

	return m_ifaceStructType->getFieldByIndexImpl (index, true);
}

StructField*
ClassType::createFieldImpl (
	const rtl::String& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	rtl::BoxList <Token>* constructor,
	rtl::BoxList <Token>* initializer
	)
{
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
		storageKind = functionKind == FunctionKind_OperatorNew ? StorageKind_Static : StorageKind_Member;

	switch (storageKind)
	{
	case StorageKind_Static:
		if (thisArgTypeFlags)
		{
			err::setFormatStringError ("static method cannot be '%s'", getPtrTypeFlagString (thisArgTypeFlags).cc ());
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

	Function** target = NULL;
	size_t overloadIdx;

	switch (functionKind)
	{
	case FunctionKind_Internal:
		return true;

	case FunctionKind_PreConstructor:
		target = &m_preConstructor;
		break;

	case FunctionKind_Constructor:
		target = &m_constructor;
		break;

	case FunctionKind_Destructor:
		target = &m_destructor;
		break;

	case FunctionKind_StaticConstructor:
		target = &m_staticConstructor;
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

	case FunctionKind_OperatorNew:
		if (!(m_flags & ClassTypeFlag_Opaque))
		{
			err::setFormatStringError (
				"'%s' is not opaque, 'operator new' is not needed",
				getTypeString ().cc ()
				);
			return false;
		}

		target = &m_operatorNew;
		break;

	case FunctionKind_Reaction:
		if (m_classTypeKind == ClassTypeKind_Reactor)
			return true;

		// else fall through and fail

	default:
		err::setFormatStringError (
			"invalid %s in '%s'",
			getFunctionKindString (functionKind),
			getTypeString ().cc ()
			);
		return false;
	}

	if (function->m_tag.isEmpty ())
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

	if (m_extensionNamespace)
		applyExtensionNamespace ();

	// resolve imports

	result =
		resolveImportBaseTypes () &&
		resolveImportFields ();

	if (!result)
		return false;

	// layout base types

	size_t baseTypeCount = m_baseTypeList.getCount ();

	char buffer [256];
	rtl::Array <BaseTypeSlot*> ifaceBaseTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	ifaceBaseTypeArray.setCount (baseTypeCount);

	rtl::Iterator <BaseTypeSlot> slotIt = m_baseTypeList.getHead ();
	for (size_t i = 0; slotIt; i++, slotIt++)
	{
		BaseTypeSlot* slot = *slotIt;

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
		if (baseClassType->m_flags & ClassTypeFlag_Opaque)
		{
			err::setFormatStringError ("cannot derive from opaque '%s'", baseClassType->getTypeString ().cc ());
			return false;
		}

		ifaceBaseTypeArray [i] = m_ifaceStructType->addBaseType (baseClassType->getIfaceStructType ());
		slot->m_VTableIndex = m_VTable.getCount ();
		m_VTable.append (baseClassType->m_VTable);
		m_pVTableStructType->append (baseClassType->m_pVTableStructType);

		m_baseTypePrimeArray.append (slot);

		if (baseClassType->m_destructor)
			m_baseTypeDestructArray.append (baseClassType);
	}

	// finalize iface layout

	result = m_ifaceStructType->ensureLayout ();
	if (!result)
		return false;

	// scan members for gcroots, constructors & destructors

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
			if (!classType->isCreatable ())
			{
				err::setFormatStringError ("cannot instantiate '%s'", type->getTypeString ().cc ());
				return false;
			}

			m_classMemberFieldArray.append (field);

			if (classType->getDestructor ())
				m_memberFieldDestructArray.append (field);
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

	// layout virtual properties

	count = m_virtualPropertyArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Property* prop = m_virtualPropertyArray [i];
		ASSERT (prop->m_storageKind == StorageKind_Abstract || prop->m_storageKind == StorageKind_Virtual);

		size_t VTableIndex = m_VTable.getCount ();

		prop->m_parentClassVTableIndex = VTableIndex;
		m_VTable.append (prop->m_VTable);
		m_pVTableStructType->append (prop->m_type->getVTableStructType ());

		size_t accessorCount = prop->m_VTable.getCount ();
		for (size_t j = 0; j < accessorCount; j++)
		{
			Function* accessor = prop->m_VTable [j];
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

	result = m_pVTableStructType->ensureLayout ();
	if (!result)
		return false;

	m_classStructType->ensureLayout ();

	createVTablePtr ();

	if (!m_staticConstructor && (m_staticDestructor || !m_initializedStaticFieldArray.isEmpty ()))
	{
		result = createDefaultMethod (FunctionKind_StaticConstructor, StorageKind_Static);
		if (!result)
			return false;
	}

	if (m_staticConstructor)
		m_staticOnceFlagVariable = m_module->m_variableMgr.createOnceFlagVariable ();

	if (m_staticDestructor)
		m_module->m_variableMgr.m_staticDestructList.addStaticDestructor (m_staticDestructor, m_staticOnceFlagVariable);

	if (!m_preConstructor &&
		(m_staticConstructor ||
		!m_ifaceStructType->getInitializedFieldArray ().isEmpty ()))
	{
		result = createDefaultMethod (FunctionKind_PreConstructor);
		if (!result)
			return false;
	}

	if (!m_constructor &&
		(m_preConstructor ||
		!m_baseTypeConstructArray.isEmpty () ||
		!m_memberFieldConstructArray.isEmpty () ||
		!m_memberPropertyConstructArray.isEmpty ()))
	{
		result = createDefaultMethod (FunctionKind_Constructor);
		if (!result)
			return false;
	}

	if (!m_destructor &&
		(!m_baseTypeDestructArray.isEmpty () ||
		!m_memberFieldDestructArray.isEmpty () ||
		!m_memberPropertyDestructArray.isEmpty ()))
	{
		result = createDefaultMethod (FunctionKind_Destructor);
		if (!result)
			return false;
	}

	if (isCreatable ())
		createPrimer ();

	m_size = m_classStructType->getSize ();
	m_alignFactor = m_classStructType->getAlignFactor ();
	return true;
}

void
ClassType::addVirtualFunction (Function* function)
{
	ASSERT (function->m_storageKind == StorageKind_Abstract || function->m_storageKind == StorageKind_Virtual);
	ASSERT (function->m_virtualOriginClassType == NULL); // not layed out yet

	function->m_virtualOriginClassType = this;
	function->m_classVTableIndex = m_VTable.getCount ();

	FunctionPtrType* pointerType = function->getType ()->getFunctionPtrType (FunctionPtrTypeKind_Thin);
	m_pVTableStructType->createField (pointerType);
	m_VTable.append (function);
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
		TraverseKind_NoExtensionNamespace | TraverseKind_NoParentNamespace | TraverseKind_NoThis
		);

	if (!member)
	{
		err::setFormatStringError ("cannot override '%s': method not found", function->m_tag.cc ());
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
				function->m_tag.cc ()
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
				err::setFormatStringError ("cannot override '%s': property has no setter", function->m_tag.cc ());
				return false;
			}

			break;

		default:
			err::setFormatStringError ("cannot override '%s': function kind mismatch", function->m_tag.cc ());
			return false;
		}

		break;

	default:
		err::setFormatStringError ("cannot override '%s': not a method or property", function->m_tag.cc ());
		return false;
	}

	overridenFunction = overridenFunction->findShortOverload (function->getType ()->getShortType ());
	if (!overridenFunction)
	{
		err::setFormatStringError ("cannot override '%s': method signature mismatch", function->m_tag.cc ());
		return false;
	}

	if (!overridenFunction->isVirtual ())
	{
		err::setFormatStringError ("cannot override '%s': method is not virtual", function->m_tag.cc ());
		return false;
	}

	#pragma AXL_TODO ("virtual multipliers")

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
		rtl::Array <FunctionArg*> argArray = function->m_type->m_argArray;
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

	size_t VTableIndex = coord.m_VTableIndex + overridenFunction->m_classVTableIndex;
	ASSERT (VTableIndex < m_VTable.getCount ());
	m_VTable [VTableIndex] = function;
	return true;
}

void
ClassType::createVTablePtr ()
{
	if (m_VTable.isEmpty ())
	{
		m_VTablePtrValue = m_pVTableStructType->getDataPtrType_c ()->getZeroValue ();
		return;
	}

	char buffer [256];
	rtl::Array <llvm::Constant*> llvmVTable (ref::BufKind_Stack, buffer, sizeof (buffer));

	size_t count = m_VTable.getCount ();
	llvmVTable.setCount (count);

	for (size_t i = 0; i < count; i++)
	{
		Function* function = m_VTable [i];
		if (function->getStorageKind () == StorageKind_Abstract)
		{
			function = function->getType ()->getAbstractFunction ();
			m_flags |= ClassTypeFlag_Abstract;
		}

		llvmVTable [i] = function->getLlvmFunction ();
	}

	llvm::Constant* llvmVTableConstant = llvm::ConstantStruct::get (
		(llvm::StructType*) m_pVTableStructType->getLlvmType (),
		llvm::ArrayRef <llvm::Constant*> (llvmVTable, count)
		);

	llvm::GlobalVariable* llvmVTableVariable = new llvm::GlobalVariable (
		*m_module->getLlvmModule (),
		m_pVTableStructType->getLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		llvmVTableConstant,
		(const char*) (m_tag + ".m_vtbl")
		);

	m_VTablePtrValue.setLlvmValue (
		llvmVTableVariable,
		m_pVTableStructType->getDataPtrType_c (),
		ValueKind_Const
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

	if (m_preConstructor && !(m_preConstructor->getFlags () & ModuleItemFlag_User))
	{
		result = compileDefaultPreConstructor ();
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

	if (m_primer)
	{
		result = compilePrimer ();
		if (!result)
			return false;
	}

	return true;
}

bool
ClassType::compileDefaultPreConstructor ()
{
	ASSERT (m_preConstructor);

	bool result;

	Value thisValue;
	m_module->m_functionMgr.internalPrologue (m_preConstructor, &thisValue, 1);

	if (m_staticConstructor)
	{
		result = m_module->m_operatorMgr.callOperator (m_staticConstructor);
		if (!result)
			return false;
	}

	result = m_ifaceStructType->initializeFields (thisValue);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

bool
ClassType::compileDefaultDestructor ()
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
ClassType::callMemberFieldDestructors (const Value& thisValue)
{
	if (m_memberFieldDestructArray.isEmpty ())
		return true;

	bool result;

	// only call member field destructors if storage is stack, static or uheap

	BasicBlock* callBlock = m_module->m_controlFlowMgr.createBlock ("call_member_field_destructors");
	BasicBlock* followBlock = m_module->m_controlFlowMgr.createBlock ("follow");

	static int32_t llvmIndexArray [] =
	{
		0, // TIfaceHdr**
		0, // TIfaceHdr*
		1, // TObjHdr**
	};

	Value objectPtrValue;
	m_module->m_llvmIrBuilder.createGep (
		thisValue,
		llvmIndexArray,
		countof (llvmIndexArray),
		NULL,
		&objectPtrValue
		);

	m_module->m_llvmIrBuilder.createLoad (objectPtrValue, NULL, &objectPtrValue);

	Type* type = m_module->getSimpleType (TypeKind_Int_p);

	Value flagsValue;
	m_module->m_llvmIrBuilder.createGep2 (objectPtrValue, 3, NULL, &flagsValue);
	m_module->m_llvmIrBuilder.createLoad (flagsValue, type, &flagsValue);

	Value maskValue;
	maskValue.setConstSizeT (ObjHdrFlag_Static | ObjHdrFlag_Stack | ObjHdrFlag_UHeap, TypeKind_Int_p);

	Value andValue;
	result =
		m_module->m_operatorMgr.binaryOperator (BinOpKind_BwAnd, flagsValue, maskValue, &andValue) &&
		m_module->m_controlFlowMgr.conditionalJump (andValue, callBlock, followBlock);

	if (!result)
		return false;

	size_t count = m_memberFieldDestructArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_memberFieldDestructArray [i];
		Type* type = field->getType ();

		ASSERT (type->getTypeKind () == TypeKind_Class);

		Function* destructor = ((ClassType*) type)->getDestructor ();
		ASSERT (destructor);

		Value fieldValue;
		result =
			m_module->m_operatorMgr.getClassField (thisValue, field, NULL, &fieldValue) &&
			m_module->m_operatorMgr.callOperator (destructor, fieldValue);

		if (!result)
			return false;
	}

	m_module->m_controlFlowMgr.follow (followBlock);
	return true;
}

bool
ClassType::callMemberPropertyDestructors (const Value& thisValue)
{
	bool result;

	size_t count = m_memberPropertyDestructArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Property* prop = m_memberPropertyDestructArray [i];

		Function* destructor = prop->getDestructor ();
		ASSERT (destructor);

		result = m_module->m_operatorMgr.callOperator (destructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

bool
ClassType::callBaseTypeDestructors (const Value& thisValue)
{
	bool result;

	size_t count = m_baseTypeDestructArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ClassType* baseClassType = m_baseTypeDestructArray [i];
		Function* destructor = baseClassType->getDestructor ();
		ASSERT (destructor);

		result = m_module->m_operatorMgr.callOperator (destructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

void
ClassType::createPrimer ()
{
	Type* argTypeArray [] =
	{
		getClassStructType ()->getDataPtrType_c (),
		m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT),  // scope level
		m_module->m_typeMgr.getStdType (StdTypeKind_ObjHdrPtr), // root
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Int_p)   // flags
	};

	FunctionType* type = m_module->m_typeMgr.getFunctionType (argTypeArray, countof (argTypeArray));
	m_primer = m_module->m_functionMgr.createFunction (FunctionKind_Primer, type);
	m_primer->m_tag = m_tag + ".prime";

	m_module->markForCompile (this);
}

bool
ClassType::compilePrimer ()
{
	ASSERT (m_primer);

	Value argValueArray [4];
	m_module->m_functionMgr.internalPrologue (m_primer, argValueArray, countof (argValueArray));

	Value argValue1 = argValueArray [0];
	Value argValue2 = argValueArray [1];
	Value argValue3 = argValueArray [2];
	Value argValue4 = argValueArray [3];

	primeObject (
		this,
		argValueArray [0],
		argValueArray [1],
		argValueArray [2],
		argValueArray [3]
		);

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

void
ClassType::primeObject (
	ClassType* classType,
	const Value& opValue,
	const Value& scopeLevelValue,
	const Value& rootValue,
	const Value& flagsValue
	)
{
	Value fieldValue;
	Value objHdrValue;
	Value ifaceValue;
	Value typeValue (&classType, m_module->m_typeMgr.getStdType (StdTypeKind_BytePtr));

	m_module->m_llvmIrBuilder.createGep2 (opValue, 0, NULL, &objHdrValue);
	m_module->m_llvmIrBuilder.createGep2 (opValue, 1, NULL, &ifaceValue);

	m_module->m_llvmIrBuilder.createGep2 (objHdrValue, 0, NULL, &fieldValue);
	m_module->m_llvmIrBuilder.createStore (scopeLevelValue, fieldValue);
	m_module->m_llvmIrBuilder.createGep2 (objHdrValue, 1, NULL, &fieldValue);
	m_module->m_llvmIrBuilder.createStore (rootValue, fieldValue);
	m_module->m_llvmIrBuilder.createGep2 (objHdrValue, 2, NULL, &fieldValue);
	m_module->m_llvmIrBuilder.createStore (typeValue, fieldValue);
	m_module->m_llvmIrBuilder.createGep2 (objHdrValue, 3, NULL, &fieldValue);
	m_module->m_llvmIrBuilder.createStore (flagsValue, fieldValue);

	primeInterface (
		classType,
		ifaceValue,
		classType->m_VTablePtrValue,
		objHdrValue,
		scopeLevelValue,
		rootValue,
		flagsValue
		);
}

void
ClassType::primeInterface (
	ClassType* classType,
	const Value& opValue,
	const Value& VTableValue,
	const Value& objectValue,
	const Value& scopeLevelValue,
	const Value& rootValue,
	const Value& flagsValue
	)
{
	// zero memory

	m_module->m_llvmIrBuilder.createStore (classType->getIfaceStructType ()->getZeroValue (), opValue);

	Value ifaceHdrValue;
	Value VTableFieldValue;
	Value objectFieldValue;

	m_module->m_llvmIrBuilder.createGep2 (opValue, 0, NULL, &ifaceHdrValue);
	m_module->m_llvmIrBuilder.createGep2 (ifaceHdrValue, 0, NULL, &VTableFieldValue);
	m_module->m_llvmIrBuilder.createGep2 (ifaceHdrValue, 1, NULL, &objectFieldValue);
	m_module->m_llvmIrBuilder.createStore (VTableValue, VTableFieldValue);
	m_module->m_llvmIrBuilder.createStore (objectValue, objectFieldValue);

	// prime base types

	size_t count = classType->m_baseTypePrimeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = classType->m_baseTypePrimeArray [i];
		ASSERT (slot->m_type->getTypeKind () == TypeKind_Class);

		ClassType* baseClassType = (ClassType*) slot->m_type;

		Value baseClassValue;
		m_module->m_llvmIrBuilder.createGep2 (
			opValue,
			slot->getLlvmIndex (),
			NULL,
			&baseClassValue
			);

		Value baseClassVTableValue;

		if (!baseClassType->hasVTable ())
		{
			baseClassVTableValue = baseClassType->getVTableStructType ()->getDataPtrType_c ()->getZeroValue ();
		}
		else
		{
			m_module->m_llvmIrBuilder.createGep2 (
				VTableValue,
				slot->getVTableIndex (),
				NULL,
				&baseClassVTableValue
				);

			m_module->m_llvmIrBuilder.createBitCast (
				baseClassVTableValue,
				baseClassType->getVTableStructType ()->getDataPtrType_c (),
				&baseClassVTableValue
				);
		}

		primeInterface (
			baseClassType,
			baseClassValue,
			baseClassVTableValue,
			objectValue,
			scopeLevelValue,
			rootValue,
			flagsValue
			);
	}

	// prime class members fields

	count = classType->m_classMemberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = classType->m_classMemberFieldArray [i];

		ASSERT (field->m_type->getTypeKind () == TypeKind_Class);
		ClassType* classType = (ClassType*) field->m_type;

		Value fieldValue;
		m_module->m_llvmIrBuilder.createGep2 (
			opValue,
			field->getLlvmIndex (),
			classType->getClassStructType ()->getDataPtrType_c (),
			&fieldValue
			);

		Function* primer = classType->getPrimer ();
		ASSERT (primer); // should have been checked during CalcLayout

		m_module->m_llvmIrBuilder.createCall4 (
			primer,
			primer->getType (),
			fieldValue,
			scopeLevelValue,
			rootValue,
			flagsValue,
			NULL
			);
	}
}

void
ClassType::gcMark (
	Runtime* runtime,
	void* p
	)
{
	ObjHdr* object = (ObjHdr*) p;
	ASSERT (object->m_type == this);

	enumGcRootsImpl (runtime, (IfaceHdr*) (object + 1));
}

void
ClassType::enumGcRootsImpl (
	Runtime* runtime,
	IfaceHdr* iface
	)
{
	char* p = (char*) iface;

	size_t count = m_gcRootBaseTypeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = m_gcRootBaseTypeArray [i];
		Type* type = slot->getType ();

		if (type->getTypeKind () == TypeKind_Class)
			((ClassType*) type)->enumGcRootsImpl (runtime, (IfaceHdr*) (p + slot->getOffset ()));
		else
			type->gcMark (runtime, p + slot->getOffset ());
	}

	count = m_gcRootMemberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_gcRootMemberFieldArray [i];
		Type* type = field->getType ();

		field->getType ()->gcMark (runtime, p + field->getOffset ());
	}
}

//.............................................................................

} // namespace jnc {
