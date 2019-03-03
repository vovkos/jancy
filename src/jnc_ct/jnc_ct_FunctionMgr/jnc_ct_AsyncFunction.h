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

#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

//..............................................................................

class AsyncFunction: public Function
{
	friend class FunctionMgr;

protected:
	ClassType* m_promiseType;

public:
	AsyncFunction()
	{
		m_functionKind = FunctionKind_Async;
		m_promiseType = NULL;
	}

	ClassType*
	getPromiseType()
	{
		return m_promiseType;
	}

	virtual
	bool
	compile();

	void
	replaceAllocas();
};

//..............................................................................

} // namespace ct
} // namespace jnc
