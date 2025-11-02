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

class EditPrivate;
class LineNumberMargin;

//..............................................................................

class JNC_EDIT_EXPORT Edit: public QPlainTextEdit {
	Q_OBJECT
	Q_DECLARE_PRIVATE(Edit)
	Q_DISABLE_COPY(Edit)
	Q_PROPERTY(bool isReadOnly READ isReadOnly WRITE setReadOnly)
	Q_PROPERTY(bool isLineNumberMarginEnabled READ isLineNumberMarginEnabled WRITE enableLineNumberMargin)
	Q_PROPERTY(int lineNumberMarginWidth READ lineNumberMarginWidth)
	Q_PROPERTY(bool isCurrentLineHighlightingEnabled READ isCurrentLineHighlightingEnabled WRITE enableCurrentLineHighlighting)
	Q_PROPERTY(bool isSyntaxHighlightingEnabled READ isSyntaxHighlightingEnabled WRITE enableSyntaxHighlighting)
	Q_PROPERTY(int tabWidth READ tabWidth WRITE setTabWidth)
	Q_PROPERTY(CodeAssistTriggers codeAssistTriggers READ codeAssistTriggers WRITE setCodeAssistTriggers)
	Q_PROPERTY(QStringList importDirList READ importDirList WRITE setImportDirList)

	friend class LineNumberMargin;

public:
	enum CodeAssistTrigger {
		QuickInfoTipOnMouseOverIdentifier      = 0x0001,
		QuickInfoTipOnCursorOverIdentifier     = 0x0002,
		ArgumentTipOnCtrlShiftSpace            = 0x0004,
		ArgumentTipOnTypeLeftParenthesis       = 0x0008,
		ArgumentTipOnTypeComma                 = 0x0010,
		ArgumentTipOnMouseOverLeftParenthesis  = 0x0020,
		ArgumentTipOnCursorOverLeftParenthesis = 0x0040,
		ArgumentTipOnMouseOverComma            = 0x0080,
		ArgumentTipOnCursorOverComma           = 0x0100,
		AutoCompleteOnCtrlSpace                = 0x0200,
		AutoCompleteOnTypeDot                  = 0x0400,
		AutoCompleteOnTypeIdentifier           = 0x0800,
		ImportAutoCompleteOnTypeQuotationMark  = 0x1000,
		GotoDefinitionOnCtrlClick              = 0x2000,
	};

	Q_DECLARE_FLAGS(CodeAssistTriggers, CodeAssistTrigger)

public:
	Edit(QWidget* parent = NULL);
	~Edit();

	// properties

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
	CodeAssistTriggers codeAssistTriggers();
	void setCodeAssistTriggers(CodeAssistTriggers triggers);
	QStringList importDirList();
	void setImportDirList(const QStringList& dirList);
	QStringList importList();
	void setImportList(const QStringList& importList);
	QString extraSource();
	void setExtraSource(const QString& source);

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
	// code-assist

	void quickInfoTip();
	void argumentTip();
	void autoComplete();
	void gotoDefinition();

	// indent

	void indentSelection();
	void unindentSelection();

protected:
	virtual void changeEvent(QEvent* e);
	virtual void resizeEvent(QResizeEvent* e);
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void enterEvent(QEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);

protected:
	QScopedPointer<EditPrivate> d_ptr;
};

//..............................................................................

Q_DECLARE_OPERATORS_FOR_FLAGS(Edit::CodeAssistTriggers)

} // namespace jnc
