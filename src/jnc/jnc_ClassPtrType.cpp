#include "pch.h"
#include "jnc_ClassPtrType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

const char*
getClassPtrTypeKindString (ClassPtrTypeKind ptrTypeKind)
{
	static const char* stringTable [ClassPtrTypeKind__Count] = 
	{
		"strong", // EClassPtrType_Normal = 0,
		"weak",   // EClassPtrType_Weak,
	};

	return (size_t) ptrTypeKind < ClassPtrTypeKind__Count ? 
		stringTable [ptrTypeKind] : 
		"undefined-class-ptr-kind";
}

//.............................................................................

ClassPtrType::ClassPtrType ()
{
	m_typeKind = TypeKind_ClassPtr;
	m_ptrTypeKind = ClassPtrTypeKind_Normal;
	m_targetType = NULL;
	m_anchorNamespace = NULL;
	m_size = sizeof (void*);
	m_alignFactor = sizeof (void*);
}

bool
ClassPtrType::isConstPtrType ()
{
	return 
		(m_flags & PtrTypeFlag_Const) != 0 || 
		(m_flags & PtrTypeFlag_ConstD) != 0 && 
		m_module->m_namespaceMgr.getAccessKind (m_anchorNamespace) == AccessKind_Public;
}

bool
ClassPtrType::isEventPtrType ()
{
	return 
		(m_flags & PtrTypeFlag_Event) != 0 || 
		(m_flags & PtrTypeFlag_EventD) != 0 && 
		m_module->m_namespaceMgr.getAccessKind (m_anchorNamespace) == AccessKind_Public;
}

rtl::String
ClassPtrType::createSignature (
	ClassType* classType,
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	rtl::String signature = typeKind == TypeKind_ClassRef ? "RC" : "PC";

	if (ptrTypeKind == ClassPtrTypeKind_Weak)
		signature += 'w';

	signature += getPtrTypeFlagSignature (flags);
	signature += classType->getSignature ();
	return signature;
}

void
ClassPtrType::prepareTypeString ()
{
	m_typeString += m_targetType->getTypeString ();

	if (m_flags & PtrTypeFlag__AllMask)
	{
		m_typeString += ' ';
		m_typeString += getPtrTypeFlagString (m_flags);
	}

	if (m_ptrTypeKind != ClassPtrTypeKind_Normal)
	{
		m_typeString += ' ';
		m_typeString += getClassPtrTypeKindString (m_ptrTypeKind);
	}

	m_typeString += m_typeKind == TypeKind_ClassRef ? "&" : "*";
}

void
ClassPtrType::prepareLlvmType ()
{
	m_llvmType = llvm::PointerType::get (m_targetType->getIfaceStructType ()->getLlvmType (), 0);
}

void
ClassPtrType::prepareLlvmDiType ()
{
	m_llvmDiType = m_module->m_llvmDiBuilder.createPointerType (m_targetType->getIfaceStructType ());
}

void
ClassPtrType::gcMark (
	Runtime* runtime,
	void* p
	)
{
	IfaceHdr* iface = *(IfaceHdr**) p;
	if (!iface || iface->m_object->m_scopeLevel)
		return;

	ObjHdr* object = iface->m_object;
	if (m_ptrTypeKind == ClassPtrTypeKind_Weak)
		object->gcWeakMarkObject ();
	else
		object->gcMarkObject (runtime);
}

//.............................................................................

} // namespace jnc {
