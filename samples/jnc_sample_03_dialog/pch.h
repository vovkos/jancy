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

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#if _WIN32
#	define _CRT_SECURE_NO_WARNINGS // disable useless warnings about "unsafe" string functions
#	define _SCL_SECURE_NO_WARNINGS // disable useless warnings about "unsafe" iterators
#	define WIN32_LEAN_AND_MEAN     // prevent winsock.h vs winsock2.h conflict
#endif

// QT

#include <QtGui>
#include <QApplication>
#include <QMainWindow>
#include <QDockWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>
#include <QList>

// Jancy

#include "jnc_Module.h"
#include "jnc_Runtime.h"
#include "jnc_CallSite.h"
#include "jnc_ExtensionLib.h"
#include "jnc_Error.h"

#if (_JNC_OS_WIN)

// Memory Leak Detection

#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>

#	ifdef _DEBUG
#		ifndef DBG_NEW
#			define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#			define new DBG_NEW
#		endif
#	endif

#elif (_JNC_OS_POSIX)
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h>
#endif
