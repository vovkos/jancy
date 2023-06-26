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
#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QToolBar>
#include <QTreeWidget>
#include <QWidget>
#include <QInputDialog>

// AXL

#include "axl_io_SockAddr.h"

#if (_JNC_IO_USB)
#	include "axl_io_UsbError.h"
#endif

#if (_JNC_IO_SSL)
#	include "axl_io_SslError.h"
#endif

#include "axl_lex_QtRagelSyntaxHighlighter.h"
#include "axl_lex_ParseError.h"
#include "axl_sys_ExceptionError.h"
#include "axl_sys_Time.h"

using namespace axl;

// Jancy

#include "jnc_Module.h"
#include "jnc_Runtime.h"
#include "jnc_Capability.h"
#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"
#include "jnc_Error.h"
#include "jnc_Edit.h"

#define LLVM_VERSION ((LLVM_VERSION_MAJOR << 16) | (LLVM_VERSION_MINOR << 8) | LLVM_VERSION_PATCH)
