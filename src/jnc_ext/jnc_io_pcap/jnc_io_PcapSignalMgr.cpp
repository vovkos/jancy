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

#include "pch.h"
#include "jnc_io_PcapSignalMgr.h"
#include "jnc_io_Pcap.h"

namespace jnc {
namespace io {

//..............................................................................

PcapSignalMgr::PcapSignalMgr()
{
	memset(&m_prevSigAction, 0, sizeof(m_prevSigAction));
	m_installCount = 0;
}

void
PcapSignalMgr::install()
{
	int32_t installCount = sys::atomicInc(&m_installCount);
	if (installCount != 1)
		return;

	struct sigaction sigAction = { 0 };
	sigAction.sa_flags = SA_SIGINFO;
	sigAction.sa_sigaction = signalHandler;
	sigemptyset(&sigAction.sa_mask);

	int result = ::sigaction(Signal, &sigAction, &m_prevSigAction);
	ASSERT(result == 0);
}

void
PcapSignalMgr::uninstall()
{
	int32_t installCount = sys::atomicDec(&m_installCount);
	if (installCount != 0)
		return;

	int result = ::sigaction(Signal, &m_prevSigAction, NULL);
	ASSERT(result == 0);

	memset(&m_prevSigAction, 0, sizeof(m_prevSigAction));
}

void
PcapSignalMgr::signalHandler(
	int signal,
	siginfo_t* signalInfo,
	void* context
	)
{
	ASSERT(signal == Signal);

	Pcap* pcap = sys::getTlsPtrSlotValue<Pcap>();
	if (pcap)
		::pcap_breakloop(pcap->getPcap());

	const struct sigaction* prevSigAction = &sl::getSimpleSingleton<PcapSignalMgr>()->m_prevSigAction;
	if (prevSigAction->sa_handler == SIG_IGN || prevSigAction->sa_handler == SIG_DFL)
		return;

	if (!(prevSigAction->sa_flags & SA_SIGINFO))
		prevSigAction->sa_handler(signal);
	else if (prevSigAction->sa_sigaction)
		prevSigAction->sa_sigaction(signal, signalInfo, context);
}

//..............................................................................

} // namespace io
} // namespace jnc
