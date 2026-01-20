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

#include <QApplication>
#include <QPlainTextEdit>
#include <QStandardItemModel>
#include <QCompleter>
#include <QAbstractItemView>
#include <QSyntaxHighlighter>
#include <QPainter>
#include <QThread>
#include <QBasicTimer>
#include <QFileIconProvider>
#include <QDirIterator>
#include <QTreeView>
#include <QScrollBar>
#include <QLabel>
#include <QStylePainter>
#include <QDesktopWidget>
#include <QStyledItemDelegate>
#include <QTextDocumentFragment>

// AXL

#include "axl_lex_QtRagelSyntaxHighlighter.h"

using namespace axl;

// Jancy

#include "jnc_Module.h"
#include "jnc_ExtensionLib.h"
#include "jnc_CodeAssist.h"
#include "jnc_EnumType.h"
#include "jnc_AttributeBlock.h"
#include "jnc_Template.h"
#include "jnc_Error.h"

// on macOS, QT flips ControlModifier and MetaModifier

#if (_AXL_OS_DARWIN)
#	define QT_CONTROL_MODIFIER Qt::MetaModifier
#else
#	define QT_CONTROL_MODIFIER Qt::ControlModifier
#endif
