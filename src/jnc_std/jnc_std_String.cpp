#include "pch.h"
#include "jnc_std_String.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//.............................................................................

JNC_DEFINE_TYPE (
	StringRef, 
	"std.StringRef", 
	g_stdLibGuid, 
	StdLibCacheSlot_StringRef
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (StringRef)
JNC_END_TYPE_FUNCTION_MAP ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	String, 
	"std.String", 
	g_stdLibGuid, 
	StdLibCacheSlot_String
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (String)
	JNC_MAP_FUNCTION ("ensureZeroTerminated", &String::ensureZeroTerminated_s)
	JNC_MAP_FUNCTION ("getZeroTerminatedString", &String::getZeroTerminatedString_s)
	JNC_MAP_FUNCTION ("copy", &String::copy_s1)
	JNC_MAP_OVERLOAD (&String::copy_s2)
JNC_END_TYPE_FUNCTION_MAP ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_CLASS_TYPE (
	StringBuilder, 
	"std.StringBuilder", 
	g_stdLibGuid, 
	StdLibCacheSlot_StringBuilder
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (StringBuilder)
	JNC_MAP_FUNCTION ("copy", &StringBuilder::copy)
	JNC_MAP_FUNCTION ("append", &StringBuilder::append)
JNC_END_TYPE_FUNCTION_MAP ()

//.............................................................................

String
String::getZeroTerminatedString ()
{
	String resultString = *this;
	resultString.ensureZeroTerminated ();
	return resultString;
}

bool
String::copy (StringRef ref)
{
	if (!ref.m_isFinal)
		return copy (ref.m_ptr, ref.m_length);

	m_ptr = ref.m_ptr;
	m_length = ref.m_length;
	return true;
}

bool
String::copy (
	DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	if (!length)
	{
		m_ptr.m_p = (void*) "";
		m_length = 0;
		return true;
	}

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	DataPtr newPtr = gcHeap->tryAllocateBuffer (length + 1);
	if (!newPtr.m_p)
		return false;

	memcpy (newPtr.m_p, ptr.m_p, length);

	m_ptr = newPtr;
	m_length = length;
	return true;
}

//.............................................................................

bool
JNC_CDECL
StringBuilder::copy (
	DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	bool result = setLength (length, false);
	if (!result)
		return false;

	memcpy (m_ptr.m_p, ptr.m_p, length);
	return true;
}

bool
JNC_CDECL
StringBuilder::append (
	DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	size_t prevLength = m_length;

	bool result = setLength (prevLength + length, true);
	if (!result)
		return false;

	memcpy ((char*) m_ptr.m_p + prevLength, ptr.m_p, length);
	return true;
}

bool
StringBuilder::setLength (
	size_t length,
	bool saveContents
	)
{
	if (length <= m_maxLength)
	{
		((char*) m_ptr.m_p) [length] = 0;
		m_length = length;
		return true;
	}

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	size_t maxLength = sl::getMinPower2Gt (length);
	DataPtr newPtr = gcHeap->tryAllocateBuffer (maxLength + 1);
	if (!newPtr.m_p)
		return false;

	if (saveContents)
		memcpy (newPtr.m_p, m_ptr.m_p, m_length);

	m_ptr = newPtr;
	m_length = length;
	m_maxLength = maxLength;
	return true;
}

//.............................................................................

} // namespace std
} // namespace jnc
