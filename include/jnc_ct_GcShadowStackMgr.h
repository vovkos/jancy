// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class Module;
class BasicBlock;
class GcShadowStackMgr;

//.............................................................................

class GcShadowStackFrameMap: public sl::ListLink
{
	friend class GcShadowStackMgr;

protected:
	GcShadowStackFrameMap* m_prev;
	sl::Array <size_t> m_gcRootIndexArray;

public:
	GcShadowStackFrameMap*
	getPrev ()
	{
		return m_prev;
	}

	const size_t*
	getGcRootIndexArray ()
	{
		return m_gcRootIndexArray.ca ();
	}

	size_t
	getGcRootCount ()
	{
		return m_gcRootIndexArray.getCount ();
	}
};

//.............................................................................

enum StackGcRootKind
{
	StackGcRootKind_Temporary,
	StackGcRootKind_Scope,
	StackGcRootKind_Function,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class GcShadowStackMgr
{
	friend class Module;

protected:
	struct RestoreFramePoint: sl::ListLink
	{
		BasicBlock* m_block;
		GcShadowStackFrameMap* m_frameMap;
	};

protected:
	Module* m_module;

	sl::Array <Type*> m_gcRootTypeArray;
	sl::StdList <GcShadowStackFrameMap> m_frameMapList;
	sl::StdList <RestoreFramePoint> m_restoreFramePointList;
	Value m_gcRootArrayValue;

	Variable* m_frameVariable;
	GcShadowStackFrameMap* m_currentFrameMap;
	Scope* m_tmpGcRootScope;

public:
	GcShadowStackMgr ();

	Module* 
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	void
	finalizeFunction ();

	void
	finalizeScope (Scope* scope);

	void
	createTmpGcRoot (const Value& value);

	void
	releaseTmpGcRoots ();

	void
	markGcRoot (
		const Value& ptrValue,
		Type* type,
		StackGcRootKind kind = StackGcRootKind_Scope,
		Scope* scope = NULL
		);

	void
	addRestoreFramePoint (
		BasicBlock* block,
		GcShadowStackFrameMap* frameMap
		);

protected:
	void
	setFrameMap (
		GcShadowStackFrameMap* frameMap,
		bool isOpen
		);

	void
	preCreateFrame ();

	void
	finalizeFrame ();

	void
	openFrameMap (Scope* scope);
};

//.............................................................................

} // namespace ct
} // namespace jnc
