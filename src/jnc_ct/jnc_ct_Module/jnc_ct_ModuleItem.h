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

#include "jnc_ModuleItem.h"
#include "jnc_ct_Lexer.h"
#include "jnc_ct_UnitMgr.h"

namespace jnc {
namespace ct {

class Module;
class Unit;
class Namespace;
class AttributeBlock;
class Type;
class DerivableType;
class ClassType;
class Function;
class Property;
struct PragmaConfig;

//..............................................................................

class ModuleItemInitializer {
	friend class Parser;

protected:
	sl::List<Token> m_initializer;

public:
	bool
	hasInitializer() const {
		return !m_initializer.isEmpty();
	}

	const sl::List<Token>&
	getInitializer() const {
		return m_initializer;
	}

	sl::StringRef
	getInitializerString() const {
		return Token::getText(m_initializer);
	}

	sl::StringRef
	getInitializerString_xml() const;
};

//..............................................................................

class ModuleItemContext {
	friend class Parser;

protected:
	Unit* m_parentUnit;
	Namespace* m_parentNamespace;

public:
	ModuleItemContext() {
		setup(NULL, NULL);
	}

	ModuleItemContext(
		Unit* parentUnit,
		Namespace* parentNamespace
	) {
		setup(parentUnit, parentNamespace);
	}

	ModuleItemContext(Module* module) {
		captureContext(module);
	}

	Unit*
	getParentUnit() const {
		return m_parentUnit;
	}

	Namespace*
	getParentNamespace() const {
		return m_parentNamespace;
	}

	Namespace*
	getGrandParentNamespace() const;

	bool
	isNullContext() const {
		return m_parentNamespace == NULL;
	}

	void
	captureContext(Module* module);

	void
	setup(
		Unit* parentUnit,
		Namespace* parentNamespace
	) {
		m_parentUnit = parentUnit;
		m_parentNamespace = parentNamespace;
	}
};

//..............................................................................

class ModuleItemPos: public ModuleItemContext {
	friend class Parser;

protected:
	lex::LineCol m_pos;

public:
	const lex::LineCol&
	getPos() const {
		return m_pos;
	}

	void
	pushSrcPosError() const {
		lex::pushSrcPosError(m_parentUnit->getFilePath(), m_pos);
	}

	void
	ensureSrcPosError() const {
		lex::ensureSrcPosError(m_parentUnit->getFilePath(), m_pos);
	}
};

//..............................................................................

class ModuleItemDecl: public ModuleItemPos {
	friend class DoxyHost;
	friend class Parser;
	friend class Namespace;
	friend class Orphan;

protected:
	StorageKind m_storageKind;
	AccessKind m_accessKind;
	sl::StringRef m_name;
	const PragmaConfig* m_pragmaConfig;
	AttributeBlock* m_attributeBlock;
	dox::Block* m_doxyBlock;

public:
	ModuleItemDecl();

	virtual
	ModuleItem*
	getDeclItem() = 0;

	StorageKind
	getStorageKind() const {
		return m_storageKind;
	}

	AccessKind
	getAccessKind() const {
		return m_accessKind;
	}

	bool
	isNamed() const {
		return !m_name.isEmpty();
	}

	const sl::StringRef&
	getName() const {
		return m_name;
	}

	const PragmaConfig*
	getPragmaConfig() const {
		return m_pragmaConfig;
	}

	AttributeBlock*
	getAttributeBlock() const {
		return m_attributeBlock;
	}

	Attribute*
	findAttribute(const sl::StringRef& name) const;

	void
	copyDecl(const ModuleItemDecl* src) {
		copyDecl(src, src->m_attributeBlock);
	}

	void
	copyDecl(
		const ModuleItemDecl* src,
		AttributeBlock* attributeBlock
	);

	dox::Block*
	getDoxyBlock() const {
		return m_doxyBlock;
	}

	bool
	ensureAttributeValuesReady();

protected:
	sl::StringRef
	createLinkIdImpl(Module* module) const;

	sl::StringRef
	createItemStringImpl(
		size_t index,
		ModuleItem* item
	) const;

	sl::StringRef
	createItemStringImpl(
		size_t index,
		ModuleItem* item,
		Type* type,
		uint_t ptrTypeFlags
	) const;

	sl::StringRef
	createQualifiedNameImpl(Module* module) const;

	sl::StringRef
	createSynopsisImpl(ModuleItem* item) const;

	sl::StringRef
	createSynopsisImpl(
		ModuleItem* item,
		Type* type,
		uint_t ptrTypeFlags
	) const;

	sl::String
	getDoxyLocationString() const {
		return m_parentUnit ?
			sl::formatString(
				"<location file='%s' line='%d' col='%d'/>\n",
				m_parentUnit->getFileName().sz(),
				m_pos.m_line + 1,
				m_pos.m_col + 1
			) :
			sl::String();
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ModuleItemDecl::ModuleItemDecl() {
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Public; // public by default
	m_pragmaConfig = NULL;
	m_attributeBlock = NULL;
	m_doxyBlock = NULL;
}

inline
void
ModuleItemDecl::copyDecl(
	const ModuleItemDecl* src,
	AttributeBlock* attributeBlock
) {
	m_parentUnit = src->m_parentUnit;
	m_parentNamespace = src->m_parentNamespace;
	m_pos = src->m_pos;
	m_storageKind = src->m_storageKind;
	m_accessKind = src->m_accessKind;
	m_name = src->m_name;
	m_pragmaConfig = src->m_pragmaConfig;
	m_attributeBlock = attributeBlock;
	m_doxyBlock = src->m_doxyBlock;
}

//..............................................................................

class ModuleItemBodyDecl: public ModuleItemDecl {
	friend class Parser;

protected:
	lex::LineColOffset m_bodyPos;
	sl::StringRef m_body;
	sl::List<Token> m_bodyTokenList;

public:
	bool
	hasBody() const {
		return !m_body.isEmpty() || !m_bodyTokenList.isEmpty();
	}

	const lex::LineColOffset&
	getBodyPos() const {
		return m_bodyPos;
	}

	const sl::StringRef&
	getBody() const {
		return m_body;
	}

	const sl::List<Token> &
	getBodyTokenList() const {
		return m_bodyTokenList;
	}

	bool
	setBody(
		const PragmaConfig* pragmaConfig,
		const lex::LineColOffset& pos,
		const sl::StringRef& body
	);

	bool
	setBody(
		const PragmaConfig* pragmaConfig,
		sl::List<Token>* tokenList
	);

	bool
	copyBody(const ModuleItemBodyDecl* srcDecl);

protected:
	bool
	canSetBody();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
ModuleItemBodyDecl::setBody(
	const PragmaConfig* pragmaConfig,
	const lex::LineColOffset& pos,
	const sl::StringRef& body
) {
	if (!canSetBody())
		return false;

	m_pragmaConfig = pragmaConfig;
	m_bodyPos = pos;
	m_body = body;
	return true;
}

inline
bool
ModuleItemBodyDecl::setBody(
	const PragmaConfig* pragmaConfig,
	sl::List<Token>* tokenList
) {
	if (!canSetBody())
		return false;

	m_pragmaConfig = pragmaConfig;
	m_bodyPos = tokenList->getHead()->m_pos;
	sl::takeOver(&m_bodyTokenList, tokenList);
	return true;
}

inline
bool
ModuleItemBodyDecl::copyBody(const ModuleItemBodyDecl* srcDecl) {
	if (!srcDecl->getBody().isEmpty())
		return setBody(srcDecl->getPragmaConfig(), srcDecl->getBodyPos(), srcDecl->getBody());

	sl::List<Token> tokenList;
	cloneTokenList(&tokenList, srcDecl->getBodyTokenList());
	setBody(srcDecl->getPragmaConfig(), &tokenList);
	return true;
}

//..............................................................................

struct ModuleItemStringCache {
	size_t m_count;
	size_t m_mask;

	// followed by space for sl::StringRef[m_count]

	ModuleItemStringCache(size_t count) {
		m_count = count;
		m_mask = 0;
	}

	~ModuleItemStringCache();

	const sl::StringRef&
	get(size_t index) const {
		ASSERT(index < m_count);
		return ((const sl::StringRef*)(this + 1))[index];
	}

	void
	set(
		size_t index,
		const sl::StringRef& string
	);

	void
	copy(const ModuleItemStringCache* cache);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ModuleItemStringCache::~ModuleItemStringCache() {
	sl::StringRef* p = (sl::StringRef*)(this + 1);
	size_t mask = m_mask;
	while (mask) {
		size_t i = sl::getLoBitIdx(mask);
		ASSERT(i < m_count);
		p[i].~StringRef();
		mask &= ~(1 << i);
	}
}

inline
void
ModuleItemStringCache::set(
	size_t index,
	const sl::StringRef& string
) {
	ASSERT(index < m_count);
	((sl::StringRef*)(this + 1))[index] = string;
	m_mask |= 1 << index;
}

inline
void
ModuleItemStringCache::copy(const ModuleItemStringCache* cache) {
	ASSERT(cache->m_count <= m_count);
	memcpy(this + 1, cache + 1, cache->m_count * sizeof(sl::StringRef));
	m_mask = cache->m_mask;
}

//..............................................................................

class ModuleItem: public sl::ListLink {
	friend class Module;
	friend class Parser;

protected:
	Module* m_module;
	sl::StringRef m_linkId;
	ModuleItemStringCache* m_stringCache;
	ModuleItemKind m_itemKind;
	uint_t m_flags;

public:
	ModuleItem();

	virtual
	~ModuleItem () {
		delete m_stringCache;
	}

	Module*
	getModule() const {
		return m_module;
	}

	ModuleItemKind
	getItemKind() const {
		return m_itemKind;
	}

	uint_t
	getFlags() const {
		return m_flags;
	}

	virtual
	ModuleItemDecl*
	getDecl() {
		return NULL;
	}

	virtual
	Namespace*
	getNamespace() {
		return NULL;
	}

	virtual
	Type*
	getItemType() {
		return NULL;
	}

	const sl::StringRef&
	getLinkId() const;

	const sl::StringRef&
	getItemString(size_t index) const;

	const sl::StringRef&
	getItemName() const {
		return getItemString(ModuleItemStringKind_QualifiedName);
	}

	virtual
	bool
	require() {
		err::setFormatStringError("don't know how to require %s", getModuleItemKindString(m_itemKind));
		return false;
	}

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	) {
		return true;
	}

	virtual
	sl::String
	createDoxyRefId();

protected:
	virtual
	sl::StringRef
	createLinkId();

	virtual
	sl::StringRef
	createItemString(size_t index) {
		return index == ModuleItemStringKind_Synopsis ?
			getModuleItemKindString(m_itemKind) :  // minimal default fallback
			sl::StringRef();
	}

	void
	prepareLinkId() {
		m_linkId = createLinkId();
		m_flags |= ModuleItemFlag_LinkIdReady;
	}

	void
	prepareItemString(size_t index);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ModuleItem::ModuleItem() {
	m_module = NULL;
	m_stringCache = NULL;
	m_itemKind = ModuleItemKind_Undefined;
	m_flags = 0;
}

inline
const sl::StringRef&
ModuleItem::getLinkId() const {
	if (!(m_flags & ModuleItemFlag_LinkIdReady))
		((ModuleItem*)this)->prepareLinkId();

	return m_linkId;
}

inline
const sl::StringRef&
ModuleItem::getItemString(size_t index) const {
	if (!m_stringCache || !(m_stringCache->m_mask & (1 << index)))
		((ModuleItem*)this)->prepareItemString(index);

	return m_stringCache->get(index);
}


//..............................................................................

template <
	typename T = ModuleItem,
	typename D = ModuleItemDecl
>
class ModuleItemWithDecl:
	public T,
	public D
{
public:
	virtual
	ModuleItem*
	getDeclItem() {
		return this;
	}

	virtual
	ModuleItemDecl*
	getDecl() {
		return this;
	}

protected:
	virtual
	sl::StringRef
	createLinkId() {
		return this->createLinkIdImpl(this->m_module);
	}

	virtual
	sl::StringRef
	createItemString(size_t index) {
		return this->createItemStringImpl(index, this); // minimal default fallback
	}
};

typedef ModuleItemWithDecl<ModuleItem, ModuleItemBodyDecl> ModuleItemWithBodyDecl;

//..............................................................................

inline
sl::StringRef
ModuleItemDecl::createItemStringImpl(
	size_t index,
	ModuleItem* item
) const{
	switch (index) {
	case ModuleItemStringKind_QualifiedName:
		return createQualifiedNameImpl(item->getModule());
	case ModuleItemStringKind_Synopsis:
		return createSynopsisImpl(item);
	default:
		return sl::StringRef();
	}
}

inline
sl::StringRef
ModuleItemDecl::createItemStringImpl(
	size_t index,
	ModuleItem* item,
	Type* type,
	uint_t ptrTypeFlags
) const{
	switch (index) {
	case ModuleItemStringKind_QualifiedName:
		return createQualifiedNameImpl(item->getModule());
	case ModuleItemStringKind_Synopsis:
		return createSynopsisImpl(item, type, ptrTypeFlags);
	default:
		return sl::StringRef();
	}
}

inline
sl::StringRef
ModuleItemDecl::createSynopsisImpl(ModuleItem* item) const {
	sl::String synopsis = getModuleItemKindString(item->getItemKind());
	synopsis += ' ';
	synopsis += item->getItemName();
	return synopsis;
}

//..............................................................................

inline
bool
ModuleItemBodyDecl::canSetBody() {
	if (!m_body.isEmpty() || !m_bodyTokenList.isEmpty()) {
		err::setFormatStringError("'%s' already has a body", getDeclItem()->getItemName().sz());
		return false;
	}

	if (m_storageKind == StorageKind_Abstract) {
		err::setFormatStringError("'%s' is abstract and hence cannot have a body", getDeclItem()->getItemName().sz());
		return false;
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
