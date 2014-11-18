#include "pch.h"
#include "jnc_PropertyPtrType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

const char*
getPropertyPtrTypeKindString (PropertyPtrTypeKind ptrTypeKind)
{
	static const char* stringTable [PropertyPtrTypeKind__Count] =
	{
		"closure",  // EPropertyPtrType_Normal = 0,
		"weak",     // EPropertyPtrType_Weak,
		"thin",     // EPropertyPtrType_Thin,
	};

	return (size_t) ptrTypeKind < PropertyPtrTypeKind__Count ?
		stringTable [ptrTypeKind] :
		"undefined-property-ptr-kind";
}

//.............................................................................

PropertyPtrType::PropertyPtrType ()
{
	m_typeKind = TypeKind_PropertyPtr;
	m_ptrTypeKind = PropertyPtrTypeKind_Normal;
	m_size = sizeof (PropertyPtr);
	m_targetType = NULL;
	m_anchorNamespace = NULL;
}

bool
PropertyPtrType::isConstPtrType ()
{
	return
		m_targetType->isReadOnly () ||
		(m_flags & PtrTypeFlag_Const) != 0 ||
		(m_flags & PtrTypeFlag_ReadOnly) != 0 &&
		m_module->m_namespaceMgr.getAccessKind (m_anchorNamespace) == AccessKind_Public;
}

StructType*
PropertyPtrType::getPropertyPtrStructType ()
{
	return m_module->m_typeMgr.getPropertyPtrStructType (m_targetType);
}

rtl::String
PropertyPtrType::createSignature (
	PropertyType* propertyType,
	TypeKind typeKind,
	PropertyPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	rtl::String signature = typeKind == TypeKind_PropertyRef ? "RX" : "PX";

	switch (ptrTypeKind)
	{
	case PropertyPtrTypeKind_Thin:
		signature += 't';
		break;

	case PropertyPtrTypeKind_Weak:
		signature += 'w';
		break;
	}

	signature += getPtrTypeFlagSignature (flags);
	signature += propertyType->getSignature ();
	return signature;
}

void
PropertyPtrType::prepareTypeString ()
{
	m_typeString = m_targetType->getReturnType ()->getTypeString ();
	m_typeString += ' ';
	m_typeString += m_targetType->getTypeModifierString ();

	if (m_flags & PtrTypeFlag__AllMask)
	{
		m_typeString += getPtrTypeFlagString (m_flags);
		m_typeString += ' ';
	}

	if (m_ptrTypeKind != PropertyPtrTypeKind_Normal)
	{
		m_typeString += getPropertyPtrTypeKindString (m_ptrTypeKind);
		m_typeString += ' ';
	}

	m_typeString += m_typeKind == TypeKind_PropertyRef ? "property&" : "property*";

	if (m_targetType->isIndexed ())
	{
		m_typeString += ' ';
		m_typeString += m_targetType->getGetterType ()->getArgString ();
	}
}

void
PropertyPtrType::prepareLlvmType ()
{
	m_llvmType =
		m_ptrTypeKind != PropertyPtrTypeKind_Thin ? getPropertyPtrStructType ()->getLlvmType () :
		m_targetType->getVTableStructType ()->getDataPtrType_c ()->getLlvmType ();
}

void
PropertyPtrType::prepareLlvmDiType ()
{
	m_llvmDiType =
		m_ptrTypeKind != PropertyPtrTypeKind_Thin ? getPropertyPtrStructType ()->getLlvmDiType () :
		m_targetType->getVTableStructType ()->getDataPtrType_c ()->getLlvmDiType ();
}

void
PropertyPtrType::gcMark (
	Runtime* runtime,
	void* p
	)
{
	ASSERT (m_ptrTypeKind == PropertyPtrTypeKind_Normal || m_ptrTypeKind == PropertyPtrTypeKind_Weak);

	PropertyPtr* ptr = (PropertyPtr*) p;
	if (!ptr->m_closure || ptr->m_closure->m_object->m_scopeLevel)
		return;

	ObjHdr* object = ptr->m_closure->m_object;
	if (m_ptrTypeKind == PropertyPtrTypeKind_Normal)
		object->gcMarkObject (runtime);
	else if (object->m_classType->getClassTypeKind () == ClassTypeKind_FunctionClosure)
		object->gcWeakMarkClosureObject (runtime);
	else  // simple weak closure
		object->gcWeakMarkObject ();
}

//.............................................................................

} // namespace jnc {
