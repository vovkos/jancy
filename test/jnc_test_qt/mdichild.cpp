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
#include "mdichild.h"
#include "jancyhighlighter.h"
#include "moc_mdichild.cpp"

#define LINE_HIGHLIGHTING_BACK		QColor(255, 255, 215)
#define LINE_NUMBER_MARGIN_BACK		QColor(255, 255, 255)
#define LINE_NUMBER_MARGIN_FORE		QColor(43, 145, 175)

MdiChild::MdiChild(QWidget *parent)
	: MdiChildBase(parent)
{
	isUntitled = true;
	isCompilationNeeded_ = true;

	setupEditor();
	setupHighlighter();
	createLineNumberMargin();
	enableLineHighlighting();
}

void MdiChild::newFile()
{
	static int sequenceNumber = 1;

	isUntitled = true;
	isCompilationNeeded_ = true;

	filePath = tr("document%1").arg(sequenceNumber++);
	setWindowTitle(filePath + "[*]");

	connect(document(), SIGNAL(contentsChanged()),
			this, SLOT(documentWasModified()));
}

bool MdiChild::loadFile(const QString& filePath)
{
	QFile file(filePath);

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::warning(this, "Open File",
							QString("Cannot read file %1:\n%2.")
								.arg(filePath)
								.arg(file.errorString()));
		return false;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QByteArray data = file.readAll();

	setPlainText(QString::fromUtf8(data));

	QApplication::restoreOverrideCursor();

	setFile(filePath);

	connect(document(), SIGNAL(contentsChanged()),
			this, SLOT(documentWasModified()));

	isCompilationNeeded_ = true;
	return true;
}

bool MdiChild::save()
{
	if (isUntitled)
		return saveAs();
	else
		return saveFile(filePath);
}

QString MdiChild::file()
{
	return this->filePath;
}

void MdiChild::closeEvent(QCloseEvent *e)
{
	if(canClose())
		e->accept();
	else
		e->ignore();
}

void MdiChild::resizeEvent(QResizeEvent *e)
{
	MdiChildBase::resizeEvent(e);

	QRect rect = contentsRect();
	lineNumberMargin->setGeometry(rect.left(), rect.top(),
		lineNumberMarginWidth, rect.height());
}

void MdiChild::documentWasModified()
{
	setWindowModified(true);
	isCompilationNeeded_ = true;
}

void MdiChild::updateLineNumberMargin(const QRect &rect,int dy)
{
	if (dy)
		lineNumberMargin->scroll(0, dy);
	else
		lineNumberMargin->update(0, rect.y(),
			lineNumberMarginWidth, rect.height());
}

void MdiChild::highlightCurrentLine()
{
	highlightSingleLine(textCursor(), LINE_HIGHLIGHTING_BACK);
}

bool MdiChild::saveAs()
{
	QString filePath = QFileDialog::getSaveFileName(this, "Save As",
							this->filePath,
							"Jancy Files (*.jnc);;All Files (*.*)");

	if (filePath.isEmpty())
		return false;

	return saveFile(filePath);
}

void MdiChild::setupEditor()
{
	setWordWrapMode(QTextOption::NoWrap);
}

void MdiChild::setupHighlighter()
{
	highlighter = new JancyHighlighter(document());
}

void MdiChild::createLineNumberMargin()
{
	lineNumberMarginWidth = fontMetrics().width(QLatin1Char('9')) * 4;
	setViewportMargins(lineNumberMarginWidth, 0, 0, 0);

	lineNumberMargin = new LineNumberMargin(this);

	QObject::connect(this, SIGNAL(updateRequest(QRect,int)),
		this, SLOT(updateLineNumberMargin(QRect,int)));
}

void MdiChild::enableLineHighlighting()
{
	QObject::connect(this, SIGNAL(cursorPositionChanged()),
		this, SLOT(highlightCurrentLine()));

	highlightCurrentLine();
}

bool MdiChild::saveFile(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Save File"),
							QString("Cannot write file %1:\n%2.")
								.arg(filePath)
								.arg(file.errorString()));
		return false;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QByteArray data = toPlainText().toUtf8();
	file.write(data);

	QApplication::restoreOverrideCursor();

	setFile(filePath);
	return true;
}

void MdiChild::setFile(const QString &filePath)
 {
	 this->filePath = QFileInfo(filePath).canonicalFilePath();

	 isUntitled = false;

	 document()->setModified(false);

	 setWindowModified(false);
	 setWindowTitle(fileName() + "[*]");
 }

QString MdiChild::fileName()
{
	return QFileInfo(filePath).fileName();
}

bool MdiChild::canClose()
{
	if (document()->isModified()) {
		QString message = QString("%1 has been modified.\n"
			"Do you want to save your changes?").arg(fileName());

		QMessageBox::StandardButton result;
		result = QMessageBox::warning(this, "Close File", message,
								QMessageBox::Save | QMessageBox::Discard |
								QMessageBox::Cancel);

		if (result == QMessageBox::Cancel)
			return false;
		else if (result == QMessageBox::Save)
			return save();
	}

	return true;
}

void MdiChild::paintLineNumberMargin(QPaintEvent *e)
{
	QPainter painter(lineNumberMargin);
	painter.fillRect(lineNumberMargin->rect(), LINE_NUMBER_MARGIN_BACK);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
	qreal bottom = top + blockBoundingRect(block).height();

	painter.setPen(LINE_NUMBER_MARGIN_FORE);

	while (block.isValid() && top <= e->rect().bottom()) {
		if (block.isVisible() && bottom >= e->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.drawText(0, (int)top, lineNumberMargin->width(),
				fontMetrics().height(), Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + blockBoundingRect(block).height();
		blockNumber++;
	}
}
