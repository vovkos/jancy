#include "pch.h"
#include "jnc_ct_ClassPtrType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//.............................................................................

ClassPtrType::ClassPtrType ()
{
	m_typeKind = TypeKind_ClassPtr;
	m_ptrTypeKind = ClassPtrTypeKind_Normal;
	m_targetType = NULL;
	m_anchorNamespace = NULL;
	m_size = sizeof (void*);
	m_alignment = sizeof (void*);
}

bool
ClassPtrType::isConstPtrType ()
{
	return 
		(m_flags & PtrTypeFlag_Const) != 0 || 
		(m_flags & PtrTypeFlag_ReadOnly) != 0 && 
		m_module->m_namespaceMgr.getAccessKind (m_anchorNamespace) == AccessKind_Public;
}

bool
ClassPtrType::isEventPtrType ()
{
	return 
		(m_flags & PtrTypeFlag_Event) != 0 || 
		(m_flags & PtrTypeFlag_DualEvent) != 0 && 
		m_module->m_namespaceMgr.getAccessKind (m_anchorNamespace) == AccessKind_Public;
}

sl::String
ClassPtrType::createSignature (
	ClassType* classType,
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	sl::String signature = typeKind == TypeKind_ClassRef ? "RC" : "PC";

	if (ptrTypeKind == ClassPtrTypeKind_Weak)
		signature += 'w';

	signature += getPtrTypeFlagSignature (flags);
	signature += classType->getSignature ();
	return signature;
}

sl::String
ClassPtrType::createTypeStringSuffix ()
{
	sl::String string;

	if (m_flags & PtrTypeFlag__AllMask)
	{
		string += ' ';
		string += getPtrTypeFlagString (m_flags);
	}

	if (m_ptrTypeKind != ClassPtrTypeKind_Normal)
	{
		string += ' ';
		string += getClassPtrTypeKindString (m_ptrTypeKind);
	}

	string += m_typeKind == TypeKind_ClassRef ? "&" : "*";
	return string;
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
ClassPtrType::markGcRoots (
	const void* p,
	rt::GcHeap* gcHeap
	)
{
	IfaceHdr* iface = *(IfaceHdr**) p;
	if (!iface)
		return;

	if (m_ptrTypeKind == ClassPtrTypeKind_Weak)
		gcHeap->weakMark (iface->m_box);
	else
		gcHeap->markClass (iface->m_box);
}

//.............................................................................

} // namespace ct
} // namespace jnc
