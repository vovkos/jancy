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

namespace jnc {
namespace ct {

class Module;

//..............................................................................

struct DfaGroupSet: sl::ListLink
{
	sl::Array <size_t> m_openArray;
	sl::Array <size_t> m_closeArray;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DfaStateInfo
{
	uintptr_t m_flags;
	DfaGroupSet* m_groupSet;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Dfa: public sl::ListLink
{
protected:
	size_t m_stateCount;
	size_t m_groupCount;
	sl::Array <uintptr_t> m_transitionTable;
	sl::Array <DfaStateInfo> m_stateInfoTable;
	sl::StdList <DfaGroupSet> m_groupSetList;

public:
	Dfa ()
	{
		m_stateCount = 0;
		m_groupCount = 0;
	}

	size_t
	getStateCount ()
	{
		return m_stateCount;
	}

	size_t
	getGroupCount ()
	{
		return m_groupCount;
	}

	bool
	build (fsm::RegExp* regExp);

	uintptr_t
	getTransition (
		uintptr_t stateId, 
		char c
		)
	{
		ASSERT (stateId < m_stateCount);
		return m_transitionTable [stateId * 256 + (uint_t) c];
	}

	DfaStateInfo*
	getStateInfo (uintptr_t stateId)
	{
		return &m_stateInfoTable [stateId];
	}
};

//.............................................................................

class AutomatonMgr
{
	friend class Module;

protected:
	Module* m_module;
	sl::StdList <Dfa> m_dfaList;

public:
	AutomatonMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ()
	{
		m_dfaList.clear ();
	}

	Dfa*
	createDfa ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
