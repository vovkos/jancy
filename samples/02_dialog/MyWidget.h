#pragma once

#include "MyLibGlobals.h"
#include "QtSignalBridge.h"

//.............................................................................

class MyWidget: public jnc::IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("Widget", g_myLibCacheSlot, MyLibTypeCacheSlot_Widget)
		JNC_MAP_AUTOGET_PROPERTY ("m_isVisible", &MyWidget::setVisible)
		JNC_MAP_AUTOGET_PROPERTY ("m_isEnabled", &MyWidget::setEnabled)
		JNC_MAP_FUNCTION ("setSizePolicy", &MyWidget::setSizePolicy)
	JNC_END_CLASS_TYPE_MAP ()

public: 
	QWidget* m_handle;
	QSizePolicy::Policy m_hpolicy;
	QSizePolicy::Policy m_vpolicy;
	bool m_isVisible;
	bool m_isEnabled;
	
public:
	MyWidget (QWidget* widget);

	void
	AXL_CDECL
	setSizePolicy (
		QSizePolicy::Policy hpolicy,
		QSizePolicy::Policy vpolicy
		);

	void
	AXL_CDECL
	setVisible (bool value);

	void
	AXL_CDECL
	setEnabled (bool value);
};

//.............................................................................
