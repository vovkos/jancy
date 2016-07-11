#include "pch.h"
#include "jnc_ct_StdExtensionLibHost.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ext {

//.............................................................................

ExtensionLibHost*
getStdExtensionLibHost ()
{
	return sl::getSingleton <StdExtensionLibHost> ();
}

//.............................................................................

size_t 
StdExtensionLibHost::getLibCacheSlot (const sl::Guid& libGuid)
{
	sl::HashTableMapIterator <sl::Guid, size_t> it = m_libSlotMap.visit (libGuid);
	if (!it->m_value)
		it->m_value = m_nextLibSlot++;
		
	return it->m_value;
}

Function*
getFunctionOverload (
	Function* function,
	size_t overloadIdx
	)
{
	Function* overload = function->getOverload (overloadIdx);
	if (!overload)
	{
		err::setFormatStringError ("'%s' has no overload #%d", function->getQualifiedName ().cc (), overloadIdx);
		return NULL;
	}

	return overload;
}

Function*
getPropertySetter (Property* prop)
{
	Function* function = prop->getSetter ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no setter", prop->m_tag.cc ());
		return NULL;
	}

	return function;
}

Function*
getTypePreConstructor (DerivableType* type)
{
	Function* function = type->getPreConstructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no preconstructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

Function*
getTypeConstructor (DerivableType* type)
{
	Function* function = type->getConstructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no constructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

Function*
getClassTypeDestructor (ClassType* type)
{
	Function* function = type->getDestructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no destructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

Function*
getTypeUnaryOperator (
	DerivableType* type,
	UnOpKind opKind	
	)
{
	Function* function = type->getUnaryOperator (opKind);
	if (!function)
	{
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), getUnOpKindString (opKind));
		return NULL;
	}

	return function;
}

Function*
getTypeBinaryOperator (
	DerivableType* type,
	BinOpKind opKind	
	)
{
	Function* function = type->getBinaryOperator (opKind);
	if (!function)
	{
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), getBinOpKindString (opKind));
		return NULL;
	}

	return function;
}

Function*
getTypeCallOperator (DerivableType* type)
{
	Function* function = type->getCallOperator ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no operator ()", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

Function*
getTypeCastOperator (
	DerivableType* type,
	size_t idx
	)
{
	Function* function = type->getCastOperator (idx);
	if (!function)
	{
		err::setFormatStringError ("'%s' has no cast operator #%d", type->getTypeString ().cc (), idx);
		return NULL;
	}

	return function;
}

//.............................................................................

} // namespace ext
} // namespace jnc


