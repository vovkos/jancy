// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Value.h"
#include "jnc_ct_LlvmIrInsertPoint.h"

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

class GcShadowStackMgr
{
	friend class Module;

protected:
	Module* m_module;

	sl::Array <Type*> m_gcRootTypeArray;
	sl::StdList <GcShadowStackFrameMap> m_frameMapList;
	Value m_gcRootArrayValue;

	Variable* m_frameVariable;
	GcShadowStackFrameMap* m_currentFrameMap;

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
	markGcRoot (
		const Value& ptrValue,
		Type* type,
		Scope* scope = NULL
		);

	void
	setFrameMap (
		GcShadowStackFrameMap* frameMap,
		bool isOpen
		);

protected:
	void
	openFrameMap (Scope* scope);

	void
	preCreateFrame ();

	void
	finalizeFrame ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
