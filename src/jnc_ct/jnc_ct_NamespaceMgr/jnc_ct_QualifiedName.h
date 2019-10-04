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

//..............................................................................

class QualifiedName
{
protected:
	sl::StringRef m_first;
	sl::BoxList<sl::StringRef> m_list;

public:
	QualifiedName()
	{
	}

	explicit
	QualifiedName(const sl::StringRef& name)
	{
		m_first = name;
	}

	QualifiedName(const QualifiedName& name)
	{
		copy(name);
	}

	QualifiedName&
	operator = (const QualifiedName& name)
	{
		copy(name);
		return *this;
	}

	void
	clear()
	{
		m_first.clear();
		m_list.clear();
	}

	void
	parse(const sl::StringRef& name);

	void
	addName(const sl::StringRef& name);

	sl::StringRef
	removeFirstName();

	sl::StringRef
	removeLastName();

	bool
	isEmpty() const
	{
		return m_first.isEmpty();
	}

	bool
	isSimple() const
	{
		return m_list.isEmpty();
	}

	const sl::StringRef&
	getFirstName() const
	{
		return m_first;
	}

	const sl::StringRef&
	getPrevName(const sl::ConstBoxIterator<sl::StringRef>& it) const
	{
		sl::ConstBoxIterator<sl::StringRef> prevIt = it.getPrev();
		return prevIt ? *prevIt : m_first;
	}

	sl::ConstBoxList<sl::StringRef>
	getNameList() const
	{
		return m_list;
	}

	const sl::StringRef&
	getShortName() const
	{
		return !m_list.isEmpty() ? *m_list.getTail() : m_first;
	}

	sl::String
	getFullName() const;

	void
	copy(const QualifiedName& name);
};

//..............................................................................

} // namespace ct
} // namespace jnc
