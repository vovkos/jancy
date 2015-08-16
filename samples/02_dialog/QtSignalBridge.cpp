#include "pch.h"
#include "QtSignalBridge.h"
#include "moc_QtSignalBridge.cpp"

//.............................................................................

QtSignalBridge::QtSignalBridge (QObject* parent):
	QObject (parent)
{
	m_runtime = jnc::getCurrentThreadRuntime ();
	ASSERT (m_runtime);

	m_jncEvent = NULL;
}

void 
QtSignalBridge::onQtSignal ()
{
	callMulticast (m_runtime, m_jncEvent);
}

//.............................................................................
