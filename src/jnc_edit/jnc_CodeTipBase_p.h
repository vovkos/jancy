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

#include "jnc_CodeTipBase.h"

namespace jnc {

//..............................................................................

class CodeTipBasePrivate: public QObject {
	Q_OBJECT
	Q_DECLARE_PUBLIC(CodeTipBase)

protected:
	CodeTipBase* q_ptr;
	const EditTheme* m_theme;
	CodeAssistKind m_codeAssistKind;
	size_t m_tipCount;
	size_t m_tipIdx;
	size_t m_argumentIdx;

protected:
	CodeTipBasePrivate() {
		m_theme = NULL;
		reset();
	}

	void
	reset();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
CodeTipBasePrivate::reset() {
	m_codeAssistKind = CodeAssistKind_None;
	m_tipCount = 0;
	m_tipIdx = 0;
	m_argumentIdx = 0;
}

//..............................................................................

} // namespace jnc
