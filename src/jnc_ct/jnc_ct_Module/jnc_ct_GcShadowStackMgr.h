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

#include "jnc_ct_Value.h"
#include "jnc_ct_LlvmIrInsertPoint.h"
#include "jnc_RuntimeStructs.h"
#include "jnc_GcHeap.h"

namespace jnc {
namespace ct {

class Module;
class BasicBlock;
class GcShadowStackMgr;

//..............................................................................

enum GcShadowStackFrameMapKind {
	GcShadowStackFrameMapKind_Static = 0,
	GcShadowStackFrameMapKind_Dynamic,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class GcShadowStackFrameMap: public sl::ListLink {
	friend class GcShadowStackMgr;

protected:
	union {
		Scope* m_scope; // before a function is finalized
		GcShadowStackFrameMap* m_prev;
	};

	GcShadowStackFrameMapKind m_mapKind;
	sl::Array<intptr_t> m_gcRootArray;
	sl::Array<Type*> m_gcRootTypeArray;

public:
	GcShadowStackFrameMap() {
		m_prev = NULL;
		m_mapKind = GcShadowStackFrameMapKind_Static;
	}

	~GcShadowStackFrameMap();

	GcShadowStackFrameMap*
	getPrev() {
		return m_prev;
	}

	GcShadowStackFrameMapKind
	getMapKind() {
		return m_mapKind;
	}

	size_t
	getGcRootCount() {
		return m_gcRootArray.getCount();
	}

	const size_t*
	getGcRootIndexArray() {
		ASSERT(m_mapKind == GcShadowStackFrameMapKind_Static);
		return (const size_t*)m_gcRootArray.cp();
	}

	Type* const*
	getGcRootTypeArray() {
		ASSERT(m_mapKind == GcShadowStackFrameMapKind_Static);
		return m_gcRootTypeArray.cp();
	}

	Box* const*
	getBoxArray() {
		ASSERT(m_mapKind == GcShadowStackFrameMapKind_Dynamic);
		return (Box* const*)m_gcRootArray.cp();
	}

	void
	addBox(Box* box) {
		ASSERT(m_mapKind == GcShadowStackFrameMapKind_Dynamic);
		m_gcRootArray.append((intptr_t)box);
	}
};

//..............................................................................

class GcShadowStackMgr {
	friend class Module;

protected:
	Module* m_module;

	sl::List<GcShadowStackFrameMap> m_frameMapList;
	Value m_gcRootArrayValue;

	sl::Array<GcShadowStackFrameMap*> m_functionFrameMapArray;
	Variable* m_frameVariable;
	size_t m_gcRootCount;

public:
	GcShadowStackMgr();

	Module*
	getModule() {
		return m_module;
	}

	bool hasFrame() {
		return m_frameVariable != NULL;
	}

	void
	clear();

	void
	finalizeFunction();

	void
	finalizeScope(Scope* scope);

	void
	createTmpGcRoot(const Value& value);

	void
	markGcRoot(
		const Value& ptrValue,
		Type* type
	);

protected:
	GcShadowStackFrameMap*
	openFrameMap(Scope* scope);

	void
	setFrameMap(
		GcShadowStackFrameMap* frameMap,
		GcShadowStackFrameMapOp op
	);

	void
	preCreateFrame();

	void
	finalizeFrame();
};

//..............................................................................

} // namespace ct
} // namespace jnc
