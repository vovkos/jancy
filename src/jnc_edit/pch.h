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

#include <QPlainTextEdit>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QToolTip>
#include <QSyntaxHighlighter>
#include <QPainter>
#include <QThread>
#include <QBasicTimer>
#include <QFileIconProvider>
#include <QDirIterator>
#include <QApplication>
#include <QTreeView>

// AXL

#include "axl_gui_QtRagelSyntaxHighlighter.h"

using namespace axl;

// Jancy

#include "jnc_Module.h"
#include "jnc_ExtensionLib.h"
#include "jnc_CodeAssist.h"
#include "jnc_Error.h"
