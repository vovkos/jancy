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

#include "jnc_Edit.h"

class MdiChild: public jnc::Edit {
	Q_OBJECT

public:
	MdiChild(QWidget *parent);

	void newFile();
	bool loadFile(const QString& filePath);
	bool save();
	bool saveAs();

	QString filePath() {
		return m_filePath;
	}

	QString fileName() {
		return QFileInfo(m_filePath).fileName();
	}

	bool isCompilationNeeded() {
		return m_isCompilationNeeded;
	}

	void setCompilationNeeded(bool isNeeded = true) {
		m_isCompilationNeeded = isNeeded;
	}

protected:
	void closeEvent(QCloseEvent *e);

private slots:
	void documentWasModified();

private:
	bool saveFile(const QString& filePath);
	void setFile(const QString &filePath);
	bool canClose();

protected:
	bool m_isUntitled;
	bool m_isCompilationNeeded;
	QString m_filePath;
};

//..............................................................................
