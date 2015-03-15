#include "pch.h"
#include "QtSignalBridge.h"
#include "moc_QtSignalBridge.cpp"

//.............................................................................

void 
QtSignalBridge::onQtSignal ()
{
	ASSERT (m_jncEvent);

	AXL_MT_BEGIN_LONG_JMP_TRY ()
	{
		m_jncEvent->call ();
	}
	AXL_MT_LONG_JMP_CATCH ()
	{
		// ignore
	}
	AXL_MT_END_LONG_JMP_TRY ()
}

//.............................................................................
