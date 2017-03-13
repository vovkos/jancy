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
class BasicBlock;

//..............................................................................

struct DfaGroupSet: sl::ListLink
{
	sl::Array <size_t> m_openArray;
	sl::Array <size_t> m_closeArray;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DfaAcceptInfo: sl::ListLink
{
	size_t m_firstGroupId;
	size_t m_groupCount;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DfaStateInfo
{
	uintptr_t m_flags;
	DfaAcceptInfo* m_acceptInfo;
	DfaGroupSet* m_groupSet;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Dfa: public sl::ListLink
{
protected:
	size_t m_stateCount;
	size_t m_groupCount;
	size_t m_maxSubMatchCount;
	sl::Array <uintptr_t> m_transitionTable;
	sl::Array <DfaStateInfo> m_stateInfoTable;
	sl::StdList <DfaAcceptInfo> m_acceptInfoList;
	sl::StdList <DfaGroupSet> m_groupSetList;

public:
	Dfa ();

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

	size_t
	getMaxSubMatchCount ()
	{
		return m_maxSubMatchCount;
	}

	bool
	build (fsm::RegEx* regEx);

	uintptr_t
	getTransition (
		uintptr_t stateId,
		uchar_t c
		)
	{
		ASSERT (stateId < m_stateCount);
		return m_transitionTable [stateId * 256 + c];
	}

	DfaStateInfo*
	getStateInfo (uintptr_t stateId)
	{
		return &m_stateInfoTable [stateId];
	}
};

//.............................................................................

class RegExMgr
{
	friend class Module;

protected:
	Module* m_module;
	sl::StdList <Dfa> m_dfaList;

public:
	RegExMgr ();

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
