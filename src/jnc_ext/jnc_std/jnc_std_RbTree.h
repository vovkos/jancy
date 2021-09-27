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

#include "jnc_std_Map.h"
#include "jnc_StdRbTree.h"

namespace jnc {
namespace std {

JNC_DECLARE_OPAQUE_CLASS_TYPE(RbTree)

typedef StdCmpFunc CmpFunc;

//..............................................................................

int
cmpVariant(
	Variant key1,
	Variant key2
);

//..............................................................................

class CmpIndirect {
protected:
	CmpFunc* m_func;

public:
	CmpIndirect(CmpFunc* func = NULL) {
		m_func = func ? func : cmpVariant;
	}

	int
	operator () (
		const Variant& key1,
		const Variant& key2
	) const {
		ASSERT(m_func);
		return m_func(key1, key2);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class RbTree: public IfaceHdr {
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(RbTree)

public:
	Map m_map;

protected:
	sl::RbTree<Variant, DataPtr, CmpIndirect> m_rbTree;

public:
	RbTree(CmpFunc* cmpFunc):
		m_rbTree(CmpIndirect(cmpFunc)) {}

	void
	JNC_CDECL
	clear() {
		m_map.clear();
		m_rbTree.clear();
	}

	static
	DataPtr
	visit(
		RbTree* self,
		Variant key
	) {
		return self->visitImpl(key);
	}

	static
	DataPtr
	find(
		RbTree* self,
		Variant key
	) {
		return self->m_rbTree.findValue(key, g_nullDataPtr);
	}

	static
	DataPtr
	findEx(
		RbTree* self,
		sl::BinTreeFindRelOp relOp,
		Variant key
	) {
		return self->m_rbTree.findValue(key, relOp, g_nullDataPtr);
	}

	void
	JNC_CDECL
	remove(DataPtr entryPtr) {
		removeImpl((MapEntry*)entryPtr.m_p);
	}

public:
	void
	removeImpl(MapEntry* entry); // used in jnc_StdRbTree_remove

protected:
	DataPtr
	visitImpl(Variant key);
};

//..............................................................................

} // namespace std
} // namespace jnc
