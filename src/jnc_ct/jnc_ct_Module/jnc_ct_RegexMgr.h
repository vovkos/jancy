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
	sl::Array<size_t> m_openArray;
	sl::Array<size_t> m_closeArray;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DfaAcceptInfo: sl::ListLink
{
	size_t m_firstGroupId;
	size_t m_groupCount;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum DfaStateFlag
{
	DfaStateFlag_Accept = 0x01,
	DfaStateFlag_Final  = 0x02,
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
	sl::Array<uintptr_t> m_transitionTable;
	sl::Array<DfaStateInfo> m_stateInfoTable;
	sl::List<DfaAcceptInfo> m_acceptInfoList;
	sl::List<DfaGroupSet> m_groupSetList;

public:
	Dfa();

	bool
	isEmpty()
	{
		return m_stateCount == 0;
	}

	size_t
	getStateCount()
	{
		return m_stateCount;
	}

	size_t
	getGroupCount()
	{
		return m_groupCount;
	}

	size_t
	getMaxSubMatchCount()
	{
		return m_maxSubMatchCount;
	}

	void
	clear();

	bool
	build(fsm::Regex* regex);

	uintptr_t
	getTransition(
		uintptr_t stateId,
		uchar_t c
		)
	{
		ASSERT(stateId < m_stateCount);
		return m_transitionTable[stateId * 256 + c];
	}

	DfaStateInfo*
	getStateInfo(uintptr_t stateId)
	{
		return &m_stateInfoTable[stateId];
	}
};

//.............................................................................

class RegexMgr
{
	friend class Module;

protected:
	Module* m_module;
	sl::List<Dfa> m_dfaList;

public:
	RegexMgr();

	Module*
	getModule()
	{
		return m_module;
	}

	void
	clear()
	{
		m_dfaList.clear();
	}

	Dfa*
	createDfa();
};

//..............................................................................

} // namespace ct
} // namespace jnc
