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

class Module;
class ModuleItem;
class ModuleItemDecl;

//..............................................................................

class DoxyHost: public dox::Host
{
protected:
	Module* m_module;

public:
	DoxyHost();

	Module*
	getModule()
	{
		return m_module;
	}

	// dox::Host

	virtual
	dox::Block*
	findItemBlock(handle_t item);

	virtual
	dox::Block*
	getItemBlock(handle_t item);

	virtual
	void
	setItemBlock(
		handle_t item,
		dox::Block* block
		);

	virtual
	sl::String
	createItemRefId(handle_t item);

	virtual
	sl::StringRef
	getItemCompoundElementName(handle_t item); // empty / innerclass / innernamespace

	virtual
	handle_t
	findItem(
		const sl::StringRef& name,
		size_t overloadIdx
		);

	virtual
	handle_t
	getCurrentNamespace();

	virtual
	bool
	generateGlobalNamespaceDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);

	// micro optimization for getItemBlock/setItemBlock

	dox::Block*
	getItemBlock(
		ModuleItem* item,
		ModuleItemDecl* itemDecl
		);

	template <typename T>
	dox::Block*
	getItemBlock(T* item)
	{
		return getItemBlock(item, item);
	}

	void
	setItemBlock(
		ModuleItem* item,
		ModuleItemDecl* itemDecl,
		dox::Block* block
		);

	template <typename T>
	void
	setItemBlock(
		T* item,
		dox::Block* block
		)
	{
		setItemBlock(item, item, block);
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
