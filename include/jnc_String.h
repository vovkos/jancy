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
		JNC_API_FUNCTION ("copy", &String::copy_s)
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

} // namespace jnc
