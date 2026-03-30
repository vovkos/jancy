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

#include "jnc_EditTheme.h"
#include "jnc_CodeAssistKind.h"

namespace jnc {

class EditBasePrivate;
class EditTheme;
class LineNumberMargin;
class HighlighterBase;
class CodeAssistThreadBase;

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
	Q_PROPERTY(bool isTabsToSpacesEnabled READ isTabsToSpacesEnabled WRITE enableTabsToSpaces)
	Q_PROPERTY(int tabWidth READ tabWidth WRITE setTabWidth)
	Q_PROPERTY(CodeAssistTriggers codeAssistTriggers READ codeAssistTriggers WRITE setCodeAssistTriggers)
	Q_PROPERTY(QStringList importDirList READ importDirList WRITE setImportDirList)
	Q_PROPERTY(QStringList importList READ importList WRITE setImportList)
	Q_PROPERTY(QString extraSource READ extraSource WRITE setExtraSource)

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

	enum CompleterColumn {
		NameColumn   = 0,
		DetailColumn = 1,
	};

	enum CompleterIcon {
		FileIcon = 0,
		ObjectIcon,
		NamespaceIcon,
		EventIcon,
		FunctionIcon,
		PropertyIcon,
		VariableIcon,
		FieldIcon,
		ConstIcon,
		TypeIcon,
		TypedefIcon,
		CompleterIconCount,
	};

	enum Role {
		CaseInsensitiveSortRole = Qt::UserRole + 1,
	};

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
	bool isTabsToSpacesEnabled();
	void enableTabsToSpaces(bool isEnabled);
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
	void indentSelection();
	void unindentSelection();
	void hideCodeAssist();
	void quickInfoTip();
	void argumentTip();
	void autoComplete();
	void gotoDefinition();

protected:
	// code assist utils

	void setActiveCodeAssist(
		CodeAssistKind codeAssistKind,
		int position
	);

	CodeAssistKind activeCodeAssistKind();
	int activeCodeAssistPosition();
	QTextCursor activeCodeAssistCursor();
	QRect activeCodeAssistCursorRect();
	QPoint activeCodeTipPoint(bool isBelowCurrentCursor = false);
	QCompleter* ensureCompleter();
	QIcon completerIcon(CompleterIcon icon);

	void updateCompleter(bool isForced = false);

	QTextCursor cursorFromLineCol(
		int line,
		int col
	);

	QTextCursor cursorFromOffset(size_t offset); // utf8 offset

	void addFile(
		QStandardItemModel* model,
		const QString& fileName
	);

protected:
	// overridables

	virtual HighlighterBase* createSyntaxHighlighter() = 0;
	virtual CodeAssistThreadBase* createCodeAssistThread() = 0;
	virtual void activateCompleter(const QModelIndex& index);
	virtual void showCodeAssist(CodeAssistThreadBase* thread) = 0;
	virtual void releaseCodeAssist() {}

	virtual void autoIndent(
		QTextCursor* cursor,
		const QString& baseIndent,
		const QString& tailWord
	);

	virtual void changeEvent(QEvent* e);
	virtual void resizeEvent(QResizeEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void enterEvent(QEvent* e);
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void keyPressPrintChar(QKeyEvent* e);

protected:
	QScopedPointer<EditBasePrivate> d_ptr;
};

//..............................................................................

Q_DECLARE_OPERATORS_FOR_FLAGS(EditBase::CodeAssistTriggers)

} // namespace jnc
