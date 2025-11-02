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
#include "moc_mdichild.cpp"

#define _DARK_THEME 0
#define _READ_ONLY  0

MdiChild::MdiChild(QWidget *parent):
	jnc::Edit(parent) {
	m_isUntitled = true;
	m_isCompilationNeeded = true;

#if (_DARK_THEME)
	setTheme(&jnc::g_defaultDarkTheme);
#endif

#if (_READ_ONLY)
	setReadOnly(true);
#endif
}

void MdiChild::newFile() {
	static int sequenceNumber = 1;

	m_isUntitled = true;
	m_isCompilationNeeded = true;

	m_filePath = tr("document%1").arg(sequenceNumber++);
	setWindowTitle(m_filePath + "[*]");

	connect(document(), SIGNAL(contentsChanged()),
			this, SLOT(documentWasModified()));
}

bool MdiChild::loadFile(const QString& filePath) {
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

	m_isCompilationNeeded = true;
	return true;
}

bool MdiChild::save() {
	if (m_isUntitled)
		return saveAs();
	else
		return saveFile(m_filePath);
}

void MdiChild::closeEvent(QCloseEvent *e) {
	if(canClose())
		e->accept();
	else
		e->ignore();
}

void MdiChild::documentWasModified() {
	setWindowModified(true);
	m_isCompilationNeeded = true;
}

bool MdiChild::saveAs() {
	QString filePath = QFileDialog::getSaveFileName(
		this,
		"Save As",
		m_filePath,
		"Jancy Files (*.jnc);;All Files (*.*)"
	);

	if (filePath.isEmpty())
		return false;

	return saveFile(filePath);
}

bool MdiChild::saveFile(const QString& filePath) {
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

void MdiChild::setFile(const QString &filePath) {
	m_filePath = QFileInfo(filePath).canonicalFilePath();
	m_isUntitled = false;

	document()->setModified(false);
	setWindowModified(false);
	setWindowTitle(fileName() + "[*]");
}

bool MdiChild::canClose() {
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
