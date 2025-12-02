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
#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_TemplateType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void
initXmlReplaceTable(sl::StringRef* table) {
	table['&'] = "&amp;";
	table['<'] = "&lt;";
	table['>'] = "&gt;";
	table['"'] = "&quot;";
	table['\''] = "&apos;";
}

sl::StringRef
ModuleItemInitializer::getInitializerString_xml() const {
	static sl::StringRef replaceTable[256] = { 0 };
	sl::callOnce(initXmlReplaceTable, replaceTable);

	sl::String originalString = getInitializerString();
	sl::String modifiedString;

	const char* p0 = originalString.cp();
	const char* end = originalString.getEnd();
	for (const char* p = p0; p < end; p++) {
		uchar_t c = *p;
		if (!replaceTable[c].isEmpty()) {
			if (p0 < p)
				modifiedString.append(p0, p - p0);

			modifiedString.append(replaceTable[c]);
			p0 = p + 1;
		}
	}

	if (modifiedString.isEmpty())
		return originalString;

	if (p0 < end)
		modifiedString.append(p0, end - p0);

	return modifiedString;
}

//..............................................................................

sl::StringRef
ModuleItemDecl::createLinkIdImpl(Module* module) const {
	sl::StringRef parentLinkId;
	if (m_parentNamespace)
		parentLinkId = m_parentNamespace->getDeclItem()->getLinkId();

	if (parentLinkId.isEmpty())
		return !m_name.isEmpty() ?
			m_name :
			module->m_namespaceMgr.getGlobalNamespace() != this ?
				sl::StringRef(module->createUniqueName("unnamed")) :
				sl::StringRef();

	sl::String linkId = parentLinkId;
	if (!m_name.isEmpty()) {
		linkId += '.';
		linkId += m_name;
	} else
		linkId.appendFormat(module->createUniqueName("unnamed"));

	return linkId;
}

sl::StringRef
ModuleItemDecl::createQualifiedNameImpl(Module* module) const {
	sl::StringRef parentName = m_parentNamespace ?
		m_parentNamespace->getDeclItem()->getItemString(ModuleItemStringKind_QualifiedName) :
		sl::StringRef();

	if (parentName.isEmpty())
		return !m_name.isEmpty() ? m_name :	"(unnamed)";

	sl::String string = parentName;
	if (m_name.isEmpty())
		string += ".(unnamed)";
	else {
		string += '.';
		string += m_name;
	}

	return string;
}

sl::StringRef
ModuleItemDecl::createSynopsisImpl(
	ModuleItem* item,
	Type* type,
	uint_t ptrTypeFlags
) const {
	type->ensureNoImports();

	sl::String synopsis;
	synopsis = type->getTypeStringPrefix();
	synopsis += ' ';

	sl::StringRef ptrTypeFlagsString = getPtrTypeFlagString(ptrTypeFlags);
	if (!ptrTypeFlagsString.isEmpty()) {
		synopsis += ptrTypeFlagsString;
		synopsis += ' ';
	}

	synopsis += item->getItemName();
	synopsis += type->getTypeStringSuffix();
	return synopsis;
}

//..............................................................................

sl::StringRef
ModuleItem::createLinkId() {
	ASSERT(false); // shouldn't be called for this item
	return sl::formatString(
		"%s.%d",
		getModuleItemKindString(m_itemKind),
		m_module->createUniqueLinkId()
	);
}

sl::String
ModuleItem::createDoxyRefId() {
	sl::String refId = getModuleItemKindString(m_itemKind);
	refId.replace('-', '_');
	refId += getLinkId();
	refId.replace('.', '_');
	refId.makeLowerCase();
	return m_module->m_doxyModule.adjustRefId(refId);
}

void
ModuleItem::prepareItemString(size_t index) {
	sl::StringRef string = createItemString(index);

	if (m_stringCache && index < m_stringCache->m_count) {
		m_stringCache->set(index, string);
		return;
	}

	size_t count = sl::align<4>(index + 1);
	size_t extra = count * sizeof(sl::StringRef);
	ModuleItemStringCache* cache = new (mem::ExtraSize(extra), mem::ZeroInit) ModuleItemStringCache(count);
	if (m_stringCache)
		cache->copy(m_stringCache);

	cache->set(index, string);
	m_stringCache = cache;
}

//..............................................................................

} // namespace ct
} // namespace jnc
