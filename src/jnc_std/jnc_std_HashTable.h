#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_Variant.h"

namespace jnc {
namespace std {

JNC_DECLARE_OPAQUE_CLASS_TYPE (StringHashTable)
JNC_DECLARE_OPAQUE_CLASS_TYPE (VariantHashTable)

//.............................................................................

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
	AXL_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	bool 
	AXL_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	AXL_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	AXL_CDECL
	find (
		DataPtr keyPtr,
		DataPtr valuePtr
		);

	void
	AXL_CDECL
	insert (
		DataPtr keyPtr,
		Variant value
		);

	bool
	AXL_CDECL
	remove (DataPtr keyPtr);
};

//.............................................................................

class VariantHashTable: public IfaceHdr
{
protected:
	struct Entry: sl::ListLink
	{
		Variant m_key;
		Variant m_value;
	};

	typedef sl::HashTableMap <Variant, Entry*, HashVariant, CmpVariant> VariantHashTableMap;

protected:
	sl::StdList <Entry> m_list;
	VariantHashTableMap m_map;

public:
	void
	AXL_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	bool
	AXL_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	AXL_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	AXL_CDECL
	find (
		Variant key,
		DataPtr valuePtr
		);

	void
	AXL_CDECL
	insert (
		Variant key,
		Variant value
		);

	bool
	AXL_CDECL
	remove (Variant key);
};

//.............................................................................

} // namespace std
} // namespace jnc
