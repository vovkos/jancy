#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace std {

JNC_DECLARE_TYPE (StringRef)
JNC_DECLARE_TYPE (String)
JNC_DECLARE_CLASS_TYPE (StringBuilder)

//..............................................................................

struct StringRef
{
	DataPtr m_ptr;
	size_t m_length;
	bool m_isFinal;
};

//..............................................................................

struct String
{
	DataPtr m_ptr;
	size_t m_length;

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
	copy (StringRef ref);

	bool
	copy (
		DataPtr ptr,
		size_t length
		);

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
	copy_s1 (
		DataPtr selfPtr,
		StringRef ref
		)
	{
		return ((String*) selfPtr.m_p)->copy (ref);
	}

	static
	bool
	copy_s2 (
		DataPtr selfPtr,
		DataPtr ptr,
		size_t length
		)
	{
		return ((String*) selfPtr.m_p)->copy (ptr, length);
	}
};

//..............................................................................

class StringBuilder: public IfaceHdr
{
public:
	DataPtr m_ptr;
	size_t m_length;
	size_t m_maxLength;

public:
	bool
	JNC_CDECL
	copy (
		DataPtr ptr,
		size_t length
		);

	bool
	JNC_CDECL
	append (
		DataPtr ptr,
		size_t length
		);

protected:
	bool
	setLength (
		size_t length,
		bool saveContents
		);
};

//..............................................................................

} // namespace std
} // namespace jnc
