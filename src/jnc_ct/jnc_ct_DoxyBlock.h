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

namespace jnc {
namespace ct {

class DoxyGroup;
class ModuleItem;

//..............................................................................

enum DoxyBlockKind
{
	DoxyBlockKind_Normal,
	DoxyBlockKind_Group,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DoxyBlock: public sl::ListLink
{
	friend class DoxyMgr;
	friend class DoxyParser;
	friend class ModuleItem;

protected:
	DoxyBlockKind m_blockKind;
	DoxyGroup* m_group;
	ModuleItem* m_item;

	sl::String m_refId;
	sl::String m_title;
	sl::String m_briefDescription;
	sl::String m_detailedDescription;
	sl::String m_seeAlsoDescription;
	sl::Array <DoxyBlock*> m_footnoteArray;
	sl::BoxList <sl::String> m_importList;

public:
	DoxyBlock ();

	DoxyBlockKind
	getBlockKind ()
	{
		return m_blockKind;
	}

	DoxyGroup*
	getGroup ()
	{
		return m_group;
	}

	ModuleItem*
	getItem ()
	{
		return m_item;
	}

	const sl::String&
	getRefId ();

	const sl::String&
	getTitle ()
	{
		return m_title;
	}

	const sl::String&
	getBriefDescription ()
	{
		return m_briefDescription;
	}

	const sl::String&
	getDetailedDescription ()
	{
		return m_detailedDescription;
	}

	const sl::String&
	getSeeAlsoDescription ()
	{
		return m_seeAlsoDescription;
	}

	bool
	isDescriptionEmpty ()
	{
		return m_briefDescription.isEmpty () && m_detailedDescription.isEmpty ();
	}

	sl::String
	getDescriptionString ();

	sl::String
	getFootnoteString ();

	sl::String
	getImportString ();

	void
	addFootnote (DoxyBlock* footnote)
	{
		m_footnoteArray.append (footnote);
	}
};

//..............................................................................

void
appendXmlElementContents (
	sl::String* string,
	const sl::StringRef& contents
	);

//..............................................................................

} // namespace ct
} // namespace jnc
