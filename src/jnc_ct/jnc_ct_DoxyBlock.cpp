#include "pch.h"
#include "jnc_ct_DoxyBlock.h"
#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_Type.h"

namespace jnc {
namespace ct {

//.............................................................................

DoxyBlock::DoxyBlock ()
{
	m_blockKind = DoxyBlockKind_Normal;
	m_group = NULL;
	m_item = NULL;
}

sl::String 
DoxyBlock::getRefId ()
{
	if (!m_refId.isEmpty ())
		return m_refId;

	ASSERT (m_item);

	m_refId = m_item->createDoxyRefId ();
	return m_refId;
}

sl::String 
DoxyBlock::getLinkedText ()
{
	if (!m_linkedText.isEmpty ())
		return m_linkedText;

	ASSERT (m_item);

	if (m_item->getItemKind () != ModuleItemKind_Type)
		m_linkedText = m_item->m_tag;
	else
		m_linkedText = ((Type*) m_item)->createDoxyLinkedText ();

	return m_linkedText;
}

inline
void
appendXmlElementContents (
	sl::String* string,
	const sl::StringRef& contents
	)
{
	if (contents.findOneOf ("<>") == -1)
	{
		string->append (contents);
	}
	else
	{
		string->append ("<![CDATA[");
		string->append (contents);
		string->append ("]]>");
	}
}

sl::String
DoxyBlock::createDescriptionString ()
{
	sl::String string;

	m_briefDescription.trim ();
	if (!m_briefDescription.isEmpty ())
	{
		string.append ("<briefdescription><para>");
		appendXmlElementContents (&string, m_briefDescription);
		string.append ("</para></briefdescription>\n");
	}

	m_detailedDescription.trim ();
	if (!m_detailedDescription.isEmpty ())
	{
		string.append ("<detaileddescription><para>");
		appendXmlElementContents (&string, m_detailedDescription);
		string.append ("</para></detaileddescription>\n");
	}

	return string;
}

//.............................................................................

} // namespace ct
} // namespace jnc
