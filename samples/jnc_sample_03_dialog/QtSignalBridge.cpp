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
#include "QtSignalBridge.h"
#include "moc_QtSignalBridge.cpp"

//..............................................................................

QtSignalBridge::QtSignalBridge (QObject* parent):
	QObject (parent)
{
	m_runtime = jnc::getCurrentThreadRuntime ();
	Q_ASSERT (m_runtime);

	m_jncEvent = NULL;
}

void
QtSignalBridge::onQtSignal ()
{
	jnc::callMulticast (m_runtime, m_jncEvent);
}

//..............................................................................
