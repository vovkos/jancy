#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {
		
//.............................................................................

struct String
{
	JNC_API_BEGIN_TYPE ("jnc.String", ApiSlot_jnc_String)
		JNC_API_FUNCTION ("ensureZeroTerminated", &String::ensureZeroTerminated_s)
		JNC_API_FUNCTION ("getZeroTerminatedString", &String::getZeroTerminatedString_s)
		JNC_API_FUNCTION_0 ("copy")
		JNC_API_OVERLOAD (&String::copy_s)
	JNC_API_END_TYPE ()

public:
	DataPtr m_ptr;
	size_t m_length;

public:
	bool 
	ensureZeroTerminated ()
	{
		return !m_ptr.m_p || ((char*) m_ptr.m_p) [m_length] != 0 ? 
			copy (m_ptr, m_length) : 
			true;
	}

	String 
	getZeroTerminatedString ();

	bool 
	copy (
		DataPtr ptr,
		size_t length
		);

protected:
	static
	bool 
	ensureZeroTerminated_s (DataPtr selfPtr)
	{
		return ((String*) selfPtr.m_p)->ensureZeroTerminated ();
	}

	static
	String 
	getZeroTerminatedString_s (DataPtr selfPtr)
	{
		return ((String*) selfPtr.m_p)->getZeroTerminatedString ();
	}

	static
	bool 
	copy_s (
		DataPtr selfPtr,
		DataPtr ptr,
		size_t length
		)
	{
		return ((String*) selfPtr.m_p)->copy (ptr, length);
	}
};

//.............................................................................

struct StringRef
{
	JNC_API_BEGIN_TYPE ("jnc.StringRef", ApiSlot_jnc_StringRef)
	JNC_API_END_TYPE ()

public:
	DataPtr m_ptr;
	size_t m_length;
	bool m_isFinal;
};

//.............................................................................

class StringBuilder: public IfaceHdr
{
public:
	JNC_API_BEGIN_CLASS ("jnc.StringBuilder", ApiSlot_jnc_StringBuilder)
		JNC_API_FUNCTION_0 ("copy")
		JNC_API_OVERLOAD (&StringBuilder::copy)
		JNC_API_FUNCTION_0 ("append")
		JNC_API_OVERLOAD (&StringBuilder::append)
	JNC_API_END_CLASS ()

public:
	DataPtr m_ptr;
	size_t m_length;
	size_t m_maxLength;

public:
	bool 
	AXL_CDECL
	copy (
		DataPtr ptr,
		size_t length
		);

	bool 
	AXL_CDECL
	append (
		DataPtr ptr,
		size_t length
		);
};

//.............................................................................

} // namespace jnc
