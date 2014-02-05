#include "pch.h"
#include "jnc_CastOp_Struct.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

ECast
CCast_Struct::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	if (OpValue.GetType ()->GetTypeKind () != EType_Struct)
		return ECast_None;

	CStructType* pStructType = (CStructType*) OpValue.GetType ();
	return pStructType->FindBaseType (pType) ? ECast_Implicit : ECast_None;
}

bool
CCast_Struct::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	if (OpValue.GetType ()->GetTypeKind () != EType_Struct)
	{
		SetCastError (OpValue, pType);
		return false;
	}

	CStructType* pStructType = (CStructType*) OpValue.GetType ();

	CBaseTypeCoord Coord;
	bool Result = pStructType->FindBaseTypeTraverse (pType, &Coord);
	if (!Result)
	{
		SetCastError (OpValue, pType);
		return false;
	}
	
	memcpy (pDst, (char*) OpValue.GetConstData () + Coord.m_Offset, pType->GetSize ());
	return true;
}

bool
CCast_Struct::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	if (OpValue.GetType ()->GetTypeKind () != EType_Struct)
	{
		SetCastError (OpValue, pType);
		return false;
	}

	CStructType* pStructType = (CStructType*) OpValue.GetType ();

	CBaseTypeCoord Coord;
	bool Result = pStructType->FindBaseTypeTraverse (pType, &Coord);
	if (!Result)
	{
		SetCastError (OpValue, pType);
		return false;
	}

	m_pModule->m_LlvmIrBuilder.CreateExtractValue (
		OpValue, 
		Coord.m_LlvmIndexArray, 
		Coord.m_LlvmIndexArray.GetCount (), 
		pType, 
		pResultValue
		);

	return true;
}

//.............................................................................

} // namespace jnc {
