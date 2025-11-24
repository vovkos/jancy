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

#include "jnc_EnumType.h"
#include "jnc_ct_Type.h"

namespace jnc {
namespace ct {

class EnumType;

//..............................................................................

const char*
getEnumTypeFlagString(EnumTypeFlag flag);

sl::StringRef
getEnumTypeFlagString(uint_t flags);

//..............................................................................

enum EnumConstFlag {
	EnumConstFlag_ValueReady = 0x0100,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class EnumConst:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer {
	friend class EnumType;
	friend class Namespace;

protected:
	EnumType* m_parentEnumType;
	int64_t m_value;
	Variable* m_declVariable;

public:
	EnumConst();

	EnumType*
	getParentEnumType() {
		return m_parentEnumType;
	}

	int64_t
	getValue() {
		return m_value;
	}

	Variable*
	getDeclVariable();

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
EnumConst::EnumConst() {
	m_itemKind = ModuleItemKind_EnumConst;
	m_parentEnumType = NULL;
	m_value = 0;
	m_declVariable = NULL;
}

//..............................................................................

class EnumType: public NamedType {
	friend class TypeMgr;
	friend class Parser;


protected:
	Type* m_rootType;
	Type* m_baseType;
	sl::List<EnumConst> m_constList;
	sl::Array<EnumConst*> m_constArray;
	sl::SimpleHashTable<int64_t, EnumConst*> m_constMap;

public:
	EnumType();

	Type*
	getBaseType() {
		return m_baseType;
	}

	Type*
	getRootType() {
		ASSERT(m_rootType);
		return m_rootType;
	}

	bool
	isBaseType(EnumType* type);

	const sl::Array<EnumConst*>&
	getConstArray() {
		return m_constArray;
	}

	EnumConst*
	createConst(
		const sl::StringRef& name,
		sl::List<Token>* initializer = NULL
	);

	EnumConst*
	findConst(int64_t value) {
		return m_constMap.findValue(value, NULL);
	}

	virtual
	sl::StringRef
	getValueString(
		const void* p,
		const char* formatSpec
	) {
		static const sl::StringRef separator(" - ");
		return getValueStringEx(separator, p, formatSpec);
	}

	sl::StringRef
	getValueName(
		const void* p,
		const char* formatSpec
	) {
		return getValueStringEx(sl::StringRef(), p, formatSpec);
	}

	sl::StringRef
	getValueStringEx(
		const sl::StringRef& separator,
		const void* p,
		const char* formatSpec
	);

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);

	virtual
	FindModuleItemResult
	findDirectChildItemTraverse(
		const sl::StringRef& name,
		MemberCoord* coord = NULL,
		uint_t flags = 0
	);

protected:
	virtual
	void
	prepareSignature();

	virtual
	void
	prepareLlvmType() {
		m_llvmType = m_baseType->getLlvmType();
	}

	virtual
	void
	prepareLlvmDiType() {
		m_llvmDiType = m_baseType->getLlvmDiType();
	}

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_EnumType);
	}

	virtual
	bool
	parseBody();

	virtual
	bool
	resolveImports() {
		return m_baseType->ensureNoImports();
	}

	virtual
	bool
	calcLayout();

	EnumConst*
	findBaseEnumConst();

	bool
	calcBitflagEnumConstValues(EnumConst* baseConst);

	bool
	calcEnumConstValues(EnumConst* baseConst);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
EnumType::EnumType() {
	m_typeKind = TypeKind_Enum;
	m_flags = TypeFlag_Pod;
	m_rootType = NULL;
	m_baseType = NULL;
}

//..............................................................................

JNC_INLINE
bool
isBitFlagEnumType(Type* type) {
	return
		type->getTypeKind() == TypeKind_Enum &&
		(((EnumType*)type)->getFlags() & EnumTypeFlag_BitFlag);
}

//..............................................................................

} // namespace ct
} // namespace jnc
