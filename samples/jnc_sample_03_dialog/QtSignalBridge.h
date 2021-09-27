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

//..............................................................................

class QtSignalBridge: public QObject {
	Q_OBJECT;

protected:
	jnc::Runtime* m_runtime;
	jnc::Multicast* m_jncEvent;

public:
	QtSignalBridge(QObject* parent = NULL);

	QtSignalBridge(
		QObject* sender,
		const char* signal,
		jnc::Multicast* jncEvent,
		QObject* parent = NULL
	):
		QObject(parent) {
		connect(sender, signal, jncEvent);
	}

	void
	connect(
		QObject* sender,
		const char* signal,
		jnc::Multicast* jncEvent
	) {
		m_jncEvent = jncEvent;
		QObject::connect(sender, signal, this, SLOT(onQtSignal()));
	}

public slots:
	void
	onQtSignal();
};

//..............................................................................
