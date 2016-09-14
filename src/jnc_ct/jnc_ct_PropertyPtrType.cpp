#include "pch.h"
#include "jnc_ct_PropertyPtrType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

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

sl::String
PropertyPtrType::createSignature (
	PropertyType* propertyType,
	TypeKind typeKind,
	PropertyPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	sl::String signature = typeKind == TypeKind_PropertyRef ? "RX" : "PX";

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

sl::String
PropertyPtrType::createDoxyLinkedText ()
{
	sl::String string = m_targetType->getReturnType ()->getDoxyBlock ()->getLinkedText ();
	string += ' ';
	string += m_targetType->getTypeModifierString ();

	if (m_flags & PtrTypeFlag__AllMask)
	{
		string += getPtrTypeFlagString (m_flags);
		string += ' ';
	}

	if (m_ptrTypeKind != PropertyPtrTypeKind_Normal)
	{
		string += getPropertyPtrTypeKindString (m_ptrTypeKind);
		string += ' ';
	}

	string += m_typeKind == TypeKind_PropertyRef ? "property&" : "property*";

	if (m_targetType->isIndexed ())
	{
		string += ' ';
		string += m_targetType->getGetterType ()->createArgDoxyLinkedText ();
	}

	return string;
}

void
PropertyPtrType::prepareLlvmType ()
{
	m_llvmType = m_ptrTypeKind != PropertyPtrTypeKind_Thin ? 
		m_module->m_typeMgr.getStdType (StdType_PropertyPtrStruct)->getLlvmType () :
		m_targetType->getVTableStructType ()->getDataPtrType_c ()->getLlvmType ();
}

void
PropertyPtrType::prepareLlvmDiType ()
{
	m_llvmDiType = m_ptrTypeKind != PropertyPtrTypeKind_Thin ? 
		m_module->m_typeMgr.getStdType (StdType_PropertyPtrStruct)->getLlvmDiType () :
		m_targetType->getVTableStructType ()->getDataPtrType_c ()->getLlvmDiType ();
}

void
PropertyPtrType::markGcRoots (
	const void* p,
	rt::GcHeap* gcHeap
	)
{
	ASSERT (m_ptrTypeKind == PropertyPtrTypeKind_Normal || m_ptrTypeKind == PropertyPtrTypeKind_Weak);

	PropertyPtr* ptr = (PropertyPtr*) p;
	if (!ptr->m_closure)
		return;

	Box* box = ptr->m_closure->m_box;
	if (m_ptrTypeKind == PropertyPtrTypeKind_Normal)
		gcHeap->markClass (box);
	else if (isClassType (box->m_type, ClassTypeKind_PropertyClosure))
		gcHeap->weakMarkClosureClass (box);
	else  // simple weak closure
		gcHeap->weakMark (box);
}

//.............................................................................

} // namespace ct
} // namespace jnc
