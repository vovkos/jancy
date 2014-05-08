#include "pch.h"
#include "jnc_Api.h"

namespace jnc {

//.............................................................................

void
PrimeInterface (
	CClassType* pType,
	TIfaceHdr* pThis,
	void* pVTable,
	TObjHdr* pObject,
	size_t ScopeLevel,
	TObjHdr* pRoot,
	uintptr_t Flags
	)
{
	pThis->m_pVTable = pVTable;
	pThis->m_pObject = pObject;

	// prime all the base types

	rtl::CArrayT <CBaseTypeSlot*> BaseTypePrimeArray = pType->GetBaseTypePrimeArray ();
	size_t Count = BaseTypePrimeArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CBaseTypeSlot* pSlot = BaseTypePrimeArray [i];
		ASSERT (pSlot->GetType ()->GetTypeKind () == EType_Class);

		PrimeInterface (
			(CClassType*) pSlot->GetType (),
			(TIfaceHdr*) ((char*) pThis + pSlot->GetOffset ()),
			pVTable ? (void**) pVTable + pSlot->GetVTableIndex () : NULL,
			pObject,
			ScopeLevel,
			pRoot,
			Flags
			);
	}

	// prime all the class fields

	rtl::CArrayT <CStructField*> FieldPrimeArray = pType->GetClassMemberFieldArray ();
	Count = FieldPrimeArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CStructField* pField = FieldPrimeArray [i];
		ASSERT (pField->GetType ()->GetTypeKind () == EType_Class);

		CClassType* pFieldType = (CClassType*) pField->GetType ();
		TObjHdr* pFieldObjHdr = (TObjHdr*) ((char*) pThis + pField->GetOffset ());
		void* pFieldVTable = NULL; // pFieldType->GetVTablePtrValue ()

		Prime (
			pFieldType, 
			pFieldVTable,
			pFieldObjHdr,
			ScopeLevel,
			pRoot,
			Flags
			);
	}
}

void
Prime (
	CClassType* pType,
	void* pVTable,
	TObjHdr* pObject,
	size_t ScopeLevel,
	TObjHdr* pRoot,
	uintptr_t Flags
	)
{
//	memset (pObject, 0, pType->GetSize ());

	pObject->m_ScopeLevel = ScopeLevel;
	pObject->m_pRoot = pRoot;
	pObject->m_pType = pType;
	pObject->m_Flags = Flags;

	PrimeInterface (pType, (TIfaceHdr*) (pObject + 1), pVTable, pObject, ScopeLevel, pRoot, Flags);
}

//.............................................................................

} // namespace jnc {
