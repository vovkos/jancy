#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_Variant.h"

namespace jnc {
namespace std {

JNC_DECLARE_OPAQUE_CLASS_TYPE (StringHashTable)
JNC_DECLARE_OPAQUE_CLASS_TYPE (VariantHashTable)

//..............................................................................

class StringHashTable: public IfaceHdr
{
protected:
	struct Entry: sl::ListLink
	{
		DataPtr m_keyPtr;
		Variant m_value;
	};

	typedef sl::StringHashTableMap <Entry*> StringHashTableMap;

protected:
	sl::StdList <Entry> m_list;
	StringHashTableMap m_map;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	bool
	JNC_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	JNC_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	JNC_CDECL
	find (
		DataPtr keyPtr,
		DataPtr valuePtr
		);

	void
	JNC_CDECL
	insert (
		DataPtr keyPtr,
		Variant value
		);

	bool
	JNC_CDECL
	remove (DataPtr keyPtr);
};

//..............................................................................

class VariantHashTable: public IfaceHdr
{
protected:
	struct Entry: sl::ListLink
	{
		Variant m_key;
		Variant m_value;
	};

	typedef sl::DuckTypeHashTableMap <Variant, Entry*> VariantHashTableMap;

protected:
	sl::StdList <Entry> m_list;
	VariantHashTableMap m_map;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	bool
	JNC_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	JNC_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	JNC_CDECL
	find (
		Variant key,
		DataPtr valuePtr
		);

	void
	JNC_CDECL
	insert (
		Variant key,
		Variant value
		);

	bool
	JNC_CDECL
	remove (Variant key);
};

//..............................................................................

} // namespace std
} // namespace jnc
