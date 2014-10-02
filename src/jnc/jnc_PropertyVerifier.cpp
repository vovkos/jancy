#include "pch.h"
#include "jnc_PropertyVerifier.h"

namespace jnc {

//.............................................................................

bool
PropertyVerifier::checkSetter (FunctionType* functionType)
{
	if (functionType->getArgArray ().isEmpty ())
	{
		err::setFormatStringError ("'set' must have at least one argument");
		return false;
	}

	return checkIndexSignature (FunctionKind_Setter, functionType);
}

bool
PropertyVerifier::checkIndexSignature (
	FunctionKind functionKind,
	FunctionType* functionType
	)
{
	ASSERT (functionKind == FunctionKind_Getter || functionKind == FunctionKind_Setter);
	
	rtl::String indexArgSignature = createIndexArgSignature (functionKind, functionType);
	if (m_indexArgSignature.isEmpty ())
	{
		m_indexArgSignature = indexArgSignature;
	}
	else if (m_indexArgSignature != indexArgSignature)
	{
		err::setFormatStringError ("index arguments mismatch in property accessors");
		return false;
	}

	return true;
}

rtl::String
PropertyVerifier::createIndexArgSignature (
	FunctionKind functionKind,
	FunctionType* functionType
	)
{
	ASSERT (functionKind == FunctionKind_Getter || functionKind == FunctionKind_Setter);

	// refine!!!

	if (functionType->isMemberMethodType ())
		functionType = functionType->getShortType ();

	if (functionKind == FunctionKind_Getter)
		return functionType->createArgSignature ();

	rtl::Array <FunctionArg*> argArray = functionType->getArgArray ();
	size_t argCount = argArray.getCount ();
	ASSERT (argCount);

	return functionType->createArgSignature (argArray, argCount - 1, 0);
}

//.............................................................................

} // namespace jnc {
