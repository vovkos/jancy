#include "pch.h"
#include "jnc_FunctionTypeOverload.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

size_t
CFunctionTypeOverload::FindOverload (CFunctionType* pType) const
{
	if (!m_pType)
		return -1;

	if (pType->Cmp (m_pType) == 0)
		return 0;

	size_t Count = m_OverloadArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CFunctionType* pOverloadType = m_OverloadArray [i];
		if (pType->Cmp (pOverloadType) == 0)
			return i + 1;
	}

	return -1;
}

size_t
CFunctionTypeOverload::FindShortOverload (CFunctionType* pType) const
{
	if (!m_pType)
		return -1;

	if (pType->Cmp (m_pType->GetShortType ()) == 0)
		return 0;

	size_t Count = m_OverloadArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CFunctionType* pOverloadType = m_OverloadArray [i];
		if (pType->Cmp (pOverloadType->GetShortType ()) == 0)
			return i + 1;
	}

	return -1;
}

size_t
CFunctionTypeOverload::ChooseOverload (
	CFunctionArg* const* pArgArray,
	size_t ArgCount,
	ECast* pCastKind
	) const
{
	ASSERT (m_pType);

	CModule* pModule = m_pType->GetModule ();

	ECast BestCastKind = pModule->m_OperatorMgr.GetArgCastKind (m_pType, pArgArray, ArgCount);
	size_t BestOverload = BestCastKind ? 0 : -1;

	size_t Count = m_OverloadArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CFunctionType* pOverloadType = m_OverloadArray [i];
		ECast CastKind = pModule->m_OperatorMgr.GetArgCastKind (pOverloadType, pArgArray, ArgCount);
		if (!CastKind)
			continue;

		if (CastKind == BestCastKind)
		{
			err::SetFormatStringError ("ambiguous call to overloaded function");
			return -1;
		}

		if (CastKind > BestCastKind)
		{
			BestOverload = i + 1;
			BestCastKind = CastKind;
		}
	}

	if (BestOverload == -1)
	{
		err::SetFormatStringError ("none of the %d overloads accept the specified argument list", Count + 1);
		return -1;
	}

	if (pCastKind)
		*pCastKind = BestCastKind;

	return BestOverload;
}

size_t
CFunctionTypeOverload::ChooseOverload (
	const rtl::CConstBoxListT <CValue>& ArgList,
	ECast* pCastKind
	) const
{
	ASSERT (m_pType);

	CModule* pModule = m_pType->GetModule ();

	ECast BestCastKind = pModule->m_OperatorMgr.GetArgCastKind (m_pType, ArgList);
	size_t BestOverload = BestCastKind ? 0 : -1;

	size_t Count = m_OverloadArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CFunctionType* pOverloadType = m_OverloadArray [i];
		ECast CastKind = pModule->m_OperatorMgr.GetArgCastKind (pOverloadType, ArgList);
		if (!CastKind)
			continue;

		if (CastKind == BestCastKind)
		{
			err::SetFormatStringError ("ambiguous call to overloaded function");
			return -1;
		}

		if (CastKind > BestCastKind)
		{
			BestOverload = i + 1;
			BestCastKind = CastKind;
		}
	}

	if (BestOverload == -1)
	{
		err::SetFormatStringError ("none of the %d overloads accept the specified argument list", Count + 1);
		return -1;
	}

	if (pCastKind)
		*pCastKind = BestCastKind;

	return BestOverload;
}

size_t
CFunctionTypeOverload::ChooseSetterOverload (
	const CValue& Value,
	ECast* pCastKind
	) const
{
	ASSERT (m_pType);

	CModule* pModule = m_pType->GetModule ();

	size_t SetterValueIdx = m_pType->GetArgArray ().GetCount () - 1;
	ASSERT (SetterValueIdx != -1);

	CType* pSetterValueArgType = m_pType->GetArgArray () [SetterValueIdx]->GetType ();
	ECast BestCastKind = pModule->m_OperatorMgr.GetCastKind (Value, pSetterValueArgType);
	size_t BestOverload = BestCastKind ? 0 : -1;

	size_t Count = m_OverloadArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CFunctionType* pOverloadType = m_OverloadArray [i];
		CType* pSetterValueArgType = pOverloadType->GetArgArray () [SetterValueIdx]->GetType ();

		ECast CastKind = pModule->m_OperatorMgr.GetCastKind (Value, pSetterValueArgType);
		if (!CastKind)
			continue;

		if (CastKind == BestCastKind)
		{
			err::SetFormatStringError ("ambiguous call to overloaded function");
			return -1;
		}

		if (CastKind > BestCastKind)
		{
			BestOverload = i + 1;
			BestCastKind = CastKind;
		}
	}

	if (BestOverload == -1)
	{
		err::SetFormatStringError ("none of the %d overloads accept the specified argument list", Count + 1);
		return -1;
	}

	if (pCastKind)
		*pCastKind = BestCastKind;

	return BestOverload;
}

bool
CFunctionTypeOverload::AddOverload (CFunctionType* pType)
{
	if (!m_pType)
	{
		m_pType = pType;
		return true;
	}
	else if (pType->GetArgSignature ().Cmp (m_pType->GetArgSignature ()) == 0)
	{
		err::SetFormatStringError ("illegal function overload: duplicate argument signature");
		return false;
	}

	size_t Count = m_OverloadArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CFunctionType* pOverloadType = m_OverloadArray [i];

		if (pType->GetArgSignature ().Cmp (pOverloadType->GetArgSignature ()) == 0)
		{
			err::SetFormatStringError ("illegal function overload: duplicate argument signature");
			return false;
		}
	}

	m_OverloadArray.Append (pType);
	return true;
}

void
CFunctionTypeOverload::Copy (
	CFunctionType* const* ppTypeArray,
	size_t Count
	)
{
	if (Count)
	{
		m_pType = ppTypeArray [0];
		m_OverloadArray.Copy (ppTypeArray + 1, Count - 1);
	}
	else
	{
		m_pType = NULL;
		m_OverloadArray.Clear ();
	}
}

bool
CFunctionTypeOverload::EnsureLayout ()
{
	bool Result;

	if (m_Flags & EModuleItemFlag_LayoutReady)
		return true;

	Result = m_pType->EnsureLayout ();
	if (!Result)
		return false;

	size_t Count = m_OverloadArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		Result = m_OverloadArray [i]->EnsureLayout ();
		if (!Result)
			return false;
	}

	m_Flags |= EModuleItemFlag_LayoutReady;
	return true;
}

//.....................................................	........................

} // namespace jnc {
