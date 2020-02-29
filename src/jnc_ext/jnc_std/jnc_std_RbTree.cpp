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

#include "pch.h"
#include "jnc_std_RbTree.h"
#include "jnc_std_StdLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace std {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	RbTree,
	"std.RbTree",
	g_stdLibGuid,
	StdLibCacheSlot_RbTree,
	RbTree,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(RbTree)
	JNC_MAP_CONSTRUCTOR(&(jnc::construct<RbTree, CmpFunc*>))
	JNC_MAP_DESTRUCTOR(&jnc::destruct<RbTree>)
	JNC_MAP_FUNCTION("clear",  &RbTree::clear)
	JNC_MAP_FUNCTION("find", &RbTree::find)
	JNC_MAP_OVERLOAD(&RbTree::findEx)
	JNC_MAP_FUNCTION("visit", &RbTree::visit)
	JNC_MAP_FUNCTION("remove", &RbTree::remove)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

int
cmpVariant(
	Variant key1,
	Variant key2
	)
{
	bool result = 0;
	return
		key1.relationalOperator(&key2, BinOpKind_Eq, &result) && result ? 0 :
		key1.relationalOperator(&key2, BinOpKind_Lt, &result) && result ? -1 : 1;
}

//..............................................................................

DataPtr
RbTree::visitImpl(Variant key)
{
	sl::MapIterator<Variant, DataPtr> it = m_rbTree.visit(key);
	if (!it->m_value.m_p)
	{
		it->m_value = m_map.add(it);
		ASSERT(m_map.m_count == m_rbTree.getCount());
	}

	return it->m_value;
}

void
RbTree::removeImpl(MapEntry* entry)
{
	if (!entry || entry->m_map != &m_map)
	{
		err::setError("attempt to remove an invalid map entry from the red-black tree");
		dynamicThrow();
	}

	m_map.remove(entry);
	m_rbTree.erase((sl::RbTreeNode<Variant, DataPtr>*) entry->m_mapEntry);
	ASSERT(m_map.m_count == m_rbTree.getCount());
}

//..............................................................................

} // namespace std
} // namespace jnc
