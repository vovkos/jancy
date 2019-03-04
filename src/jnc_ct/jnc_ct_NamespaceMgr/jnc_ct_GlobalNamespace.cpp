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
#include "jnc_ct_GlobalNamespace.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

sl::String
GlobalNamespace::createDoxyRefId()
{
	sl::String refId;

	if (this == m_module->m_namespaceMgr.getGlobalNamespace())
	{
		refId = "global_namespace";
	}
	else
	{
		refId.format("namespace_%s", getQualifiedName().sz());
		refId.makeLowerCase();
	}

	return m_module->m_doxyMgr.adjustRefId(refId);
}

bool
GlobalNamespace::generateDocumentation(
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	DoxyBlock* doxyBlock = m_module->m_doxyMgr.getDoxyBlock(this);

	const char* kind;
	const char* name;

	if (this == m_module->m_namespaceMgr.getGlobalNamespace())
	{
		kind = "file";
		name = "global";
	}
	else
	{
		kind = "namespace";
		name = getQualifiedName();
	}

	indexXml->appendFormat(
		"<compound kind='%s' refid='%s'><name>%s</name></compound>\n",
		kind,
		doxyBlock->getRefId().sz(),
		name
		);

	itemXml->format(
		"<compounddef kind='%s' id='%s' language='Jancy'>\n"
		"<compoundname>%s</compoundname>\n",
		kind,
		doxyBlock->getRefId().sz(),
		name
		);

	sl::String memberXml;
	Namespace::generateMemberDocumentation(outputDir, &memberXml, indexXml, true);
	itemXml->append(memberXml);

	sl::String footnoteXml = doxyBlock->getFootnoteString();
	if (!footnoteXml.isEmpty())
	{
		itemXml->append("<sectiondef>\n");
		itemXml->append(footnoteXml);
		itemXml->append("</sectiondef>\n");
	}

	itemXml->append(doxyBlock->getDescriptionString());
	itemXml->append(getDoxyLocationString());
	itemXml->append("</compounddef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
