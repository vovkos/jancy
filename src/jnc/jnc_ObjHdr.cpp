#include "pch.h"
#include "jnc_ObjHdr.h"
#include "jnc_ClosureClassType.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

void 
TObjHdr::GcMarkData (CRuntime* pRuntime)
{
	m_pRoot->m_Flags |= EObjHdrFlag_GcWeakMark;

	if (m_Flags & EObjHdrFlag_GcRootsAdded)
		return;

	m_Flags |= EObjHdrFlag_GcRootsAdded;

	if (!(m_pType->GetFlags () & ETypeFlag_GcRoot))
		return;

	if (!(m_Flags & EObjHdrFlag_DynamicArray))
	{
		if (m_pType->GetTypeKind () == EType_Class)
			pRuntime->AddGcRoot (this, m_pType);
		else
			pRuntime->AddGcRoot (this + 1, m_pType);
	}
	else
	{
		ASSERT (m_pType->GetTypeKind () != EType_Class);

		char* p = (char*) (this + 1);		
		size_t Count = *((size_t*) this - 1);
		for (size_t i = 0; i < Count; i++)
		{
			pRuntime->AddGcRoot (p, m_pType);
			p += m_pType->GetSize  ();
		}
	}
}

void 
TObjHdr::GcMarkObject (CRuntime* pRuntime)
{
	m_pRoot->m_Flags |= EObjHdrFlag_GcWeakMark;
	m_Flags |= EObjHdrFlag_GcMark;

	if (m_Flags & EObjHdrFlag_GcRootsAdded)
		return;

	m_Flags |= EObjHdrFlag_GcRootsAdded;

	if (!(m_pType->GetFlags () & ETypeFlag_GcRoot))
		return;

	pRuntime->AddGcRoot (this, m_pType);
}

void
TObjHdr::GcWeakMarkClosureObject (CRuntime* pRuntime)
{
	m_pRoot->m_Flags |= EObjHdrFlag_GcWeakMark;
	m_Flags |= EObjHdrFlag_GcMark;

	if (m_Flags & (EObjHdrFlag_GcWeakMark_c | EObjHdrFlag_GcRootsAdded))
		return;

	m_Flags |= EObjHdrFlag_GcWeakMark_c;

	CClosureClassType* pClosureClassType = (CClosureClassType*) m_pClassType;
	if (!pClosureClassType->GetWeakMask ())
	{
		GcMarkObject (pRuntime);
		return;
	}

	char* p = (char*) (this + 1);

	rtl::CArrayT <CStructField*> GcRootMemberFieldArray = pClosureClassType->GetGcRootMemberFieldArray ();
	size_t Count = GcRootMemberFieldArray.GetCount ();

	for (size_t i = 0; i < Count; i++)
	{
		CStructField* pField = GcRootMemberFieldArray [i];
		CType* pType = pField->GetType ();
		ASSERT (pType->GetFlags () & ETypeFlag_GcRoot);		

		if (pField->GetFlags () & EStructFieldFlag_WeakMasked)
			pType = GetWeakPtrType (pType);

		pType->GcMark (pRuntime, p + pField->GetOffset ());
	}
}

//.............................................................................

} // namespace jnc {
