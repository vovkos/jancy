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

#include <QColor>

namespace jnc {

#if (_JNC_EDIT_DLL)
#  define JNC_EDIT_EXPORT Q_DECL_EXPORT
#else
#  define JNC_EDIT_EXPORT Q_DECL_IMPORT
#endif

//..............................................................................

class JNC_EDIT_EXPORT EditTheme {
public:
	enum Init {
		InitNone,
		InitLight,
		InitDark,
	};

	enum Role {
		// editor elements

		BaseBackDisabled,
		BaseBack,
		BaseText,
		LineMarginBack,
		LineMarginText,
		ErrorBack,
		ErrorText,
		SelectionBack,
		SelectionText,
		SelectionBackInactive,
		SelectionTextInactive,
		BraceMatchBack,
		BraceMatchText,
		CurrentLineBack,
		CompleterSynopsisColumn,

		// syntax highlighting

		Keyword,
		Constant,
		Comment,

		RoleCount,
	};

public:
	EditTheme(Init init = InitLight);

	bool isDark() const {
		return m_colorTable[BaseBack].value() < 0x80;
	}

	QColor color(Role role) const {
		ASSERT((size_t)role < RoleCount);
		return m_colorTable[role];
	}

	void setColor(
		Role role,
		const QColor& color
	) {
		m_colorTable[role] = color;
		invalidatePalette();
	}

	const QPalette& palette() {
		return m_palette.color(QPalette::Base).isValid() ? m_palette : createPalette();
	}

	const QPalette& readOnlyPalette() {
		return m_readOnlyPalette.color(QPalette::Base).isValid() ? m_readOnlyPalette : createReadOnlyPalette();
	}

	void setDefaultLightTheme();
	void setDefaultDarkTheme();

protected:
	void invalidatePalette() {
		m_palette.setColor(QPalette::Base, QColor::Invalid);
		m_readOnlyPalette.setColor(QPalette::Base, QColor::Invalid);
	}

	const QPalette& createPalette();
	const QPalette& createReadOnlyPalette();

	static void setPaletteColor(
		QPalette* palette,
		QPalette::ColorGroup group,
		QPalette::ColorRole role,
		const QColor& color
	);

	static void setPaletteColor(
		QPalette* palette,
		QPalette::ColorRole role,
		const QColor& color
	) {
		setPaletteColor(palette, QPalette::All, role, color);
	}

	void setPaletteColor(
		QPalette::ColorGroup group,
		QPalette::ColorRole role,
		const QColor& color
	) {
		setPaletteColor(&m_palette, group, role, color);
	}

	void setPaletteColor(
		QPalette::ColorRole role,
		const QColor& color
	) {
		setPaletteColor(&m_palette, QPalette::All, role, color);
	}

protected:
	QColor m_colorTable[RoleCount];
	mutable QPalette m_palette;
	mutable QPalette m_readOnlyPalette;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
EditTheme::EditTheme(Init init) {
	switch (init) {
	case InitLight:
		setDefaultLightTheme();
		break;

	case InitDark:
		setDefaultDarkTheme();
		break;
	}
}

inline
void EditTheme::setPaletteColor(
	QPalette* palette,
	QPalette::ColorGroup group,
	QPalette::ColorRole role,
	const QColor& color
) {
	if (color.isValid())
		palette->setColor(group, role, color);
	else
		palette->setBrush(group, role, Qt::NoBrush);
}
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SELECT_ANY EditTheme g_defaultLightTheme(EditTheme::InitLight);
AXL_SELECT_ANY EditTheme g_defaultDarkTheme(EditTheme::InitDark);

//..............................................................................

} // namespace jnc
