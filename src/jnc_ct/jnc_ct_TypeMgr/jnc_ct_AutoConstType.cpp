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
#include "jnc_ct_AutoConstType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

sl::StringRef
AutoConstType::createItemString(size_t index) {
	if (!(m_flags & TypeFlag_Dual)) // mutable adapter
		return m_originalType->getItemString(index);

	sl::String string;
	switch (index) {
	case ModuleItemStringKind_Synopsis:
		string = "template ";

	case ModuleItemStringKind_QualifiedName:
	case TypeStringKind_Prefix:
		string += "jnc.AutoConst<";
		string += m_originalType->getItemName();
		string += ", ";
		string += m_constType->getItemName();
		string += ">";
		break;

	default:
		return Type::createItemString(index);
	}

	return string;
}

bool
AutoConstType::calcLayout() {
	bool result =
		m_originalType->ensureLayout() &&
		m_constType->ensureLayout();

	if (!result)
		return false;

	if (!m_originalType->isLayoutIdentical(m_constType)) {
		err::setError("jnc.AutoConst<T, C> arguments must have identical data layouts");
		return NULL;
	}

	if (!(m_originalType->getTypeKindFlags() & TypeKindFlag_Nullable)) {
		err::setError("jnc.AutoConst<T, C> arguments must be value types");
		return NULL;
	}

	m_alignment = m_constType->getAlignment();
	m_size = m_constType->getSize();
	m_flags |= m_constType->getFlags() &
		jnc_TypeFlag_Pod |
		jnc_TypeFlag_GcRoot |
		jnc_TypeFlag_StructRet |
		jnc_TypeFlag_NoStack;

	return true;
}

Type*
AutoConstType::calcFoldedDualType(
	bool isAlien,
	uint_t ptrFlags
) {
	ASSERT(m_flags & TypeFlag_Dual);

	// we can't simply return m_originalType because underlying llvm type is from m_constType
	// so we create an adapter that's llvm-compatible and then casted to m_originalType during prepareOperand

	return (ptrFlags & (PtrTypeFlag_Const | PtrTypeFlag_MaybeConst)) ?
		m_constType :
		m_module->m_typeMgr.createAutoConstType(m_originalType, m_constType, 0);
}

//..............................................................................

} // namespace ct
} // namespace jnc
