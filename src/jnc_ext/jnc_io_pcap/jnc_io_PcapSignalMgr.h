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
namespace io {

//..............................................................................

class PcapSignalMgr
{
public:
	enum
	{
		Signal = SIGUSR2
	};

protected:
	struct sigaction m_prevSigAction;
	volatile int32_t m_installCount;

public:
	PcapSignalMgr();

	~PcapSignalMgr()
	{
		ASSERT(m_installCount == 0);
	}

	void
	install();

	void
	uninstall();

protected:
	static
	void
	signalHandler(
		int signal,
		siginfo_t* signalInfo,
		void* context
		);
};

//..............................................................................

} // namespace io
} // namespace jnc
