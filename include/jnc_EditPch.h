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

#include <QPlainTextEdit>
#include <QPalette>

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (_JNC_EDIT_DLL)
#  define JNC_EDIT_EXPORT Q_DECL_EXPORT
#else
#  define JNC_EDIT_EXPORT Q_DECL_IMPORT
#endif

//..............................................................................
