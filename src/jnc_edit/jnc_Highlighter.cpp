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
#include "jnc_Highlighter.h"
#include "jnc_Highlighter.rl.cpp"

namespace jnc {

//..............................................................................

QString
highlightJancySource(const QString& source) {
	QTextDocument document;
	document.setPlainText(source);

	JancyHighlighter highlighter(&document);
	highlighter.rehighlight();

	QString html;

	QTextBlock block = document.firstBlock();
	for (; block.isValid(); block = block.next()) {
		QString blockText = block.text();

#if (QT_VERSION >= 0x050600)
		QVector<QTextLayout::FormatRange> formats = block.layout()->formats();
#else
		QList<QTextLayout::FormatRange> formats = block.layout()->additionalFormats();
#endif

		int pos = 0;
		int count = formats.count();
		for (int i = 0; i < count; i++) {
			QTextLayout::FormatRange range = formats[i];
			uint_t color = range.format.foreground().color().rgb();
			if (!color)
				continue;

			if (range.start > pos)
				html += blockText.mid(pos, range.start - pos);

			html += QString("<span style='color: #%1'>").arg(color, 6, 16, QChar('0'));
			html += blockText.mid(range.start, range.length);
			html += "</span>";

			pos = range.start + range.length;
		}

		int blockEnd = block.position() + block.length();

		if (blockEnd > pos)
			html += blockText.mid(pos);

		pos = blockEnd;
	}

	return html;
}

//..............................................................................

} // namespace jnc
