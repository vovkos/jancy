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

#include <QPlainTextEdit>
#include "jnc_EditTheme.h"

namespace jnc {

class LineNumberMargin;
class HighlighterBase;
class EditBasePrivate;

//..............................................................................

class JNC_EDIT_EXPORT EditBase: public QPlainTextEdit {
	Q_OBJECT
	Q_DECLARE_PRIVATE(EditBase)
	Q_DISABLE_COPY(EditBase)
	Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
	Q_PROPERTY(bool isReadOnly READ isReadOnly WRITE setReadOnly)
	Q_PROPERTY(bool isLineNumberMarginEnabled READ isLineNumberMarginEnabled WRITE enableLineNumberMargin)
	Q_PROPERTY(int lineNumberMarginWidth READ lineNumberMarginWidth)
	Q_PROPERTY(bool isCurrentLineHighlightingEnabled READ isCurrentLineHighlightingEnabled WRITE enableCurrentLineHighlighting)
	Q_PROPERTY(bool isSyntaxHighlightingEnabled READ isSyntaxHighlightingEnabled WRITE enableSyntaxHighlighting)
	Q_PROPERTY(int tabWidth READ tabWidth WRITE setTabWidth)

	friend class LineNumberMargin;

protected:
	EditBase(
		QWidget* parent,
		EditBasePrivate* d
	);

public:
	EditBase(QWidget* parent);
	~EditBase(); // otherwise QScopedPointer<EditBasePrivate> would produce incomplete type error

	// properties

	QString fileName();
	void setFileName(const QString& fileName);
	void setReadOnly(bool isReadOnly);
	bool isLineNumberMarginEnabled();
	void enableLineNumberMargin(bool isEnabled);
	int lineNumberMarginWidth();
	bool isCurrentLineHighlightingEnabled();
	void enableCurrentLineHighlighting(bool isEnabled);
	bool isSyntaxHighlightingEnabled();
	void enableSyntaxHighlighting(bool isEnabled);
	int tabWidth();
	void setTabWidth(int width);
	const EditTheme* theme();
	void setTheme(const EditTheme* theme);

	// selection/highlighth operations

	void setTextCursorLineCol(
		int line,
		int col
	);

	void highlightLineTemp(
		int line,
		const QColor& backColor,
		const QColor& textColor = QColor::Invalid
	);

public slots:
	void indentSelection();
	void unindentSelection();

protected:
	virtual HighlighterBase* createSyntaxHighlighter() = 0; // define in subclass

	virtual void changeEvent(QEvent* e);
	virtual void resizeEvent(QResizeEvent* e);
	virtual void keyPressEvent(QKeyEvent* e);

protected:
	QScopedPointer<EditBasePrivate> d_ptr;
};

//..............................................................................

} // namespace jnc
