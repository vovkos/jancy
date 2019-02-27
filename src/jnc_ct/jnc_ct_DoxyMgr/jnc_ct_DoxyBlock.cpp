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
#include "jnc_ct_DoxyBlock.h"
#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_Type.h"

namespace jnc {
namespace ct {

//..............................................................................

DoxyBlock::DoxyBlock()
{
	m_blockKind = DoxyBlockKind_Normal;
	m_group = NULL;
	m_item = NULL;
}

const sl::String&
DoxyBlock::getRefId()
{
	if (!m_refId.isEmpty())
		return m_refId;

	ASSERT(m_item);

	m_refId = m_item->createDoxyRefId();
	return m_refId;
}

sl::String
DoxyBlock::getDescriptionString()
{
	sl::String string;

	m_briefDescription.trim();
	m_detailedDescription.trim();
	m_seeAlsoDescription.trim();

	if (!m_briefDescription.isEmpty())
	{
		string.append("<briefdescription><para>");
		appendXmlElementContents(&string, m_briefDescription);
		string.append("</para></briefdescription>\n");
	}

	if (!m_detailedDescription.isEmpty() ||
		!m_seeAlsoDescription.isEmpty() ||
		!m_internalDescription.isEmpty()
		)
	{
		string.append("<detaileddescription>\n");

		if (!m_detailedDescription.isEmpty())
		{
			string.append("<para>");
			appendXmlElementContents(&string, m_detailedDescription);
			string.append("</para>\n");
		}

		if (!m_seeAlsoDescription.isEmpty())
		{
			string.append("<para><simplesect kind='see'><para>");
			appendXmlElementContents(&string, m_seeAlsoDescription);
			string.append("</para></simplesect></para>\n");
		}

		if (!m_internalDescription.isEmpty())
		{
			string.append("<internal><para>");
			appendXmlElementContents(&string, m_internalDescription);
			string.append("</para></internal>\n");
		}

		string.append("</detaileddescription>\n");
	}

	return string;
}

sl::String
DoxyBlock::getFootnoteString()
{
	sl::String string;

	size_t count = m_footnoteArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		DoxyBlock* footnote = m_footnoteArray[i];

		string.append("<memberdef kind='footnote'>\n");
		string.appendFormat("<name>%s</name>\n", footnote->getRefId ().sz ());
		string.append(footnote->getDescriptionString());
		string.append("</memberdef>\n");
	}

	return string;
}

sl::String
DoxyBlock::getImportString()
{
	sl::String string;

	sl::BoxIterator<sl::String> it = m_importList.getHead();
	for (; it; it++)
		string.appendFormat("<includes>%s</includes>\n", it->sz ());

	return string;
}


//..............................................................................

void
appendXmlElementContents(
	sl::String* string,
	const sl::StringRef& contents
	)
{
	if (contents.findOneOf("<>&") == -1)
	{
		string->append(contents);
	}
	else
	{
		string->append("<![CDATA[");
		string->append(contents);
		string->append("]]>");
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
