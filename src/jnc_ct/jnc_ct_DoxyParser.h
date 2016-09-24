// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Def.h"

namespace jnc {
namespace ct {

class DoxyGroup;
class DoxyBlock;

//.............................................................................

class DoxyParser
{
protected:
	struct GroupStackEntry
	{
		Namespace* m_namespace;
		DoxyGroup* m_group;
	};

protected:
	Module* m_module;

	DoxyBlock* m_block;
	sl::Array <GroupStackEntry> m_groupStack;
	bool m_isBlockAssigned;
	bool m_isBriefDescription;
	sl::String m_firstIndent;
	sl::String m_indent;

public:
	DoxyParser (Module* module);

	DoxyBlock*
	popBlock ();

	void
	addComment (
		const sl::StringRef& comment,
		const lex::LineCol& lineCol,
		bool canAppend
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
