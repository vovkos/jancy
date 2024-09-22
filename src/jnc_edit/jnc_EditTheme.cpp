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
#include "jnc_EditTheme.h"

namespace jnc {

//..............................................................................

void EditTheme::setDefaultLightTheme() {
	m_colorTable[BaseBackDisabled]        = 0xf0f0f0;
	m_colorTable[BaseBack]                = 0xffffff;
	m_colorTable[BaseText]                = (QRgb)0x000000;
	m_colorTable[LineMarginBack]          = 0xf0f0f0;
	m_colorTable[LineMarginText]          = 0x2b91af;
	m_colorTable[ErrorBack]               = 0xffc8c8;
	m_colorTable[ErrorText]               = QColor::Invalid;
	m_colorTable[SelectionBack]           = 0x99c9ef;
	m_colorTable[SelectionText]           = QColor::Invalid;
	m_colorTable[SelectionBackInactive]   = 0xe0e0e0;
	m_colorTable[SelectionTextInactive]   = QColor::Invalid;
	m_colorTable[BraceMatchBack]          = 0xfff080;
	m_colorTable[BraceMatchText]          = QColor::Invalid;
	m_colorTable[CurrentLineBack]         = 0xe8eff8;
	m_colorTable[CompleterSynopsisColumn] = 0x808080;

	m_colorTable[Keyword]  = 0x0000ff;
	m_colorTable[Constant] = 0xce7b00;
	m_colorTable[Comment]  = 0x969696;

	invalidatePalette();
}

void EditTheme::setDefaultDarkTheme() {
	// based on monokai

	m_colorTable[BaseBackDisabled]        = 0x3e4851;
	m_colorTable[BaseBack]                = 0x2e3841;
	m_colorTable[BaseText]                = 0xd7dee9;
	m_colorTable[LineMarginBack]          = 0x3e4851;
	m_colorTable[LineMarginText]          = 0x838b95;
	m_colorTable[ErrorBack]               = 0x773f40;
	m_colorTable[ErrorText]               = QColor::Invalid;
	m_colorTable[SelectionBack]           = 0x405672;
	m_colorTable[SelectionText]           = QColor::Invalid;
	m_colorTable[SelectionBackInactive]   = 0x505050;
	m_colorTable[SelectionTextInactive]   = QColor::Invalid;
	m_colorTable[BraceMatchBack]          = 0x835c42;
	m_colorTable[BraceMatchText]          = QColor::Invalid;
	m_colorTable[CurrentLineBack]         = 0x3e4851;
	m_colorTable[CompleterSynopsisColumn] = 0x7d7d7d;

	m_colorTable[Keyword]  = 0xca95c5;
	m_colorTable[Constant] = 0x94c796;
	m_colorTable[Comment]  = 0xa5acb8;

	invalidatePalette();
}

const QPalette& EditTheme::createPalette() const {
	setPaletteColor(QPalette::Base, m_colorTable[BaseBack]);
	setPaletteColor(QPalette::Window, m_colorTable[BaseBack]);
	setPaletteColor(QPalette::Text, m_colorTable[BaseText]);
	setPaletteColor(QPalette::WindowText, m_colorTable[BaseText]);
	setPaletteColor(QPalette::Highlight, m_colorTable[SelectionBack]);
	setPaletteColor(QPalette::HighlightedText, m_colorTable[SelectionText]);

	setPaletteColor(QPalette::Inactive, QPalette::Highlight, m_colorTable[SelectionBackInactive]);
	setPaletteColor(QPalette::Inactive, QPalette::HighlightedText, m_colorTable[SelectionTextInactive]);

	setPaletteColor(QPalette::Disabled, QPalette::Base, m_colorTable[BaseBackDisabled]);
	setPaletteColor(QPalette::Disabled, QPalette::Window, m_colorTable[BaseBackDisabled]);
	setPaletteColor(QPalette::Disabled, QPalette::Highlight, m_colorTable[SelectionBackInactive]);
	setPaletteColor(QPalette::Disabled, QPalette::HighlightedText, m_colorTable[SelectionTextInactive]);
	return m_palette;
}

const QPalette& EditTheme::createCompleterPalette() const {
	m_completerPalette = palette();
	setPaletteColor(&m_completerPalette, QPalette::HighlightedText, m_colorTable[BaseText]);
	return m_completerPalette;
}

const QPalette& EditTheme::createReadOnlyPalette() const {
	m_readOnlyPalette = palette();
	setPaletteColor(&m_readOnlyPalette, QPalette::Base, m_colorTable[BaseBackDisabled]);
	setPaletteColor(&m_readOnlyPalette, QPalette::Inactive, QPalette::Base, m_colorTable[BaseBackDisabled]);
	return m_readOnlyPalette;
}

//..............................................................................

} // namespace jnc
