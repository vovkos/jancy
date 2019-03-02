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

class BasicBlock;

//..............................................................................

struct ReSwitchAcceptContext: sl::ListLink
{
	size_t m_firstGroupId;
	size_t m_groupCount;

	union
	{
		BasicBlock* m_actionBlock;
		size_t m_actionIdx;
	};
};

//..............................................................................

} // namespace ct
} // namespace jnc
