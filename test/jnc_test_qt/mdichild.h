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

#ifndef _MDICHILD_H
#define _MDICHILD_H

#include "editor.h"

class LineNumberMargin;
class JancyHighlighter;

#define MdiChildBase Editor

class MdiChild : public MdiChildBase
{
	Q_OBJECT

public:
	MdiChild(QWidget *parent);

	void newFile();
	bool loadFile(const QString& filePath);
	bool save();
	bool saveAs();

	QString file();

	bool isCompilationNeeded ()
	{
		return isCompilationNeeded_;
	}

	void setCompilationNeeded (bool isNeeded = true)
	{
		isCompilationNeeded_ = isNeeded;
	}

protected:
	void closeEvent(QCloseEvent *e);
	void resizeEvent(QResizeEvent *e);

private slots:
	void documentWasModified();
	void updateLineNumberMargin(const QRect &rect,int dy);
	void highlightCurrentLine();

private:
	void setupEditor();
	void setupHighlighter();
	void createLineNumberMargin();
	void enableLineHighlighting();
	bool saveFile(const QString& filePath);
	void setFile(const QString &filePath);
	QString fileName();
	bool canClose();
	void paintLineNumberMargin(QPaintEvent *e);

	bool isUntitled;
	bool isCompilationNeeded_;
	QString filePath;
	int lineNumberMarginWidth;
	LineNumberMargin *lineNumberMargin;
	JancyHighlighter *highlighter;

	friend class LineNumberMargin;
};

//..............................................................................

class LineNumberMargin : public QWidget
{
public:
	LineNumberMargin(MdiChild *editor) : QWidget (editor)
		{ this->editor = editor; }

	QSize sizeHint() const
		{ return QSize (editor->lineNumberMarginWidth, 0); }

protected:
	void paintEvent(QPaintEvent *e)
		{ editor->paintLineNumberMargin(e); }

private:
	MdiChild *editor;
};

#endif
