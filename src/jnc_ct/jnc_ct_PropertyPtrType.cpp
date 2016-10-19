#include "pch.h"
#include "jnc_ct_PropertyPtrType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//..............................................................................

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

//..............................................................................

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
	TypeStringTuple* tuple = getTypeStringTuple ();
	Type* returnType = m_targetType->getReturnType ();

	tuple->m_typeStringPrefix = returnType->getTypeStringPrefix ();

	if (m_flags & PtrTypeFlag__AllMask)
	{
		tuple->m_typeStringPrefix += ' ';
		tuple->m_typeStringPrefix += getPtrTypeFlagString (m_flags);
	}

	if (m_ptrTypeKind != PropertyPtrTypeKind_Normal)
	{
		tuple->m_typeStringPrefix += ' ';
		tuple->m_typeStringPrefix += getPropertyPtrTypeKindString (m_ptrTypeKind);
	}

	tuple->m_typeStringPrefix += m_typeKind == TypeKind_PropertyRef ? " property&" : " property*";

	if (m_targetType->isIndexed ())
		tuple->m_typeStringSuffix += m_targetType->getGetterType ()->getTypeStringSuffix ();

	tuple->m_typeStringSuffix += returnType->getTypeStringSuffix ();
}

void
PropertyPtrType::prepareDoxyLinkedText ()
{
	TypeStringTuple* tuple = getTypeStringTuple ();
	Type* returnType = m_targetType->getReturnType ();

	tuple->m_doxyLinkedTextPrefix = returnType->getDoxyLinkedTextPrefix ();

	if (m_flags & PtrTypeFlag__AllMask)
	{
		tuple->m_doxyLinkedTextPrefix += ' ';
		tuple->m_doxyLinkedTextPrefix += getPtrTypeFlagString (m_flags);
	}

	if (m_ptrTypeKind != PropertyPtrTypeKind_Normal)
	{
		tuple->m_doxyLinkedTextPrefix += ' ';
		tuple->m_doxyLinkedTextPrefix += getPropertyPtrTypeKindString (m_ptrTypeKind);
	}

	tuple->m_doxyLinkedTextPrefix += m_typeKind == TypeKind_PropertyRef ? " property&" : " property*";

	if (m_targetType->isIndexed ())
		tuple->m_doxyLinkedTextSuffix += m_targetType->getGetterType ()->getDoxyLinkedTextSuffix ();

	tuple->m_doxyLinkedTextSuffix += returnType->getDoxyLinkedTextSuffix ();
}

void
PropertyPtrType::prepareDoxyTypeString ()
{
	Type::prepareDoxyTypeString ();

	if (m_targetType->isIndexed ())
		getTypeStringTuple ()->m_doxyTypeString += m_targetType->getGetterType ()->getDoxyArgString ();
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

//..............................................................................

} // namespace ct
} // namespace jnc
