//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(DynamicLayout)

//..............................................................................

class DynamicLayout: public IfaceHdr
{
protected:
	struct Key
	{
		void* m_base;
		DerivableType* m_type;

		size_t hash() const
		{
			return sl::djb2(this, sizeof(Key));
		}

		bool isEqual(const Key& key) const
		{
			return m_base == key.m_base && m_type == key.m_type;
		}
	};

	struct Entry: sl::ListLink
	{
		sl::Array<size_t> m_endOffsetArray;
	};

protected:
	sys::Lock m_lock;
	sl::DuckTypeHashTable<Key, Entry*> m_map;
	sl::List<Entry> m_list;

public:
	static
	ClassType*
	getType(Module* module);

	size_t
	getDynamicFieldSize(
		DataPtr ptr,
		size_t offset,
		Field* field
		);

	size_t
	getDynamicFieldEndOffset(
		DataPtr ptr,
		DerivableType* type,
		size_t fieldIndex // dynamic
		);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
