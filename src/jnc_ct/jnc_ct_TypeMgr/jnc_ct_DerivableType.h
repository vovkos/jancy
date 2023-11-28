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

#include "jnc_ct_MemberBlock.h"

namespace jnc {
namespace ct {

class DerivableType;
class StructType;
class UnionType;
class ClassType;
class Function;
class Property;

//..............................................................................

class BaseTypeSlot:
	public ModuleItem,
	public ModuleItemDecl {
	friend class DerivableType;
	friend class StructType;
	friend class ClassType;

protected:
	DerivableType* m_type;
	size_t m_offset;
	size_t m_vtableIndex;
	uint_t m_llvmIndex;

public:
	BaseTypeSlot();

	uint_t
	getFlags() const {
		return m_flags;
	}

	DerivableType*
	getType() const {
		return m_type;
	}

	size_t
	getOffset() const {
		return m_offset;
	}

	size_t
	getVtableIndex() const {
		return m_vtableIndex;
	}

	uint_t
	getLlvmIndex() const {
		return m_llvmIndex;
	}
};

//..............................................................................

class BaseTypeCoord {
	AXL_DISABLE_COPY(BaseTypeCoord)

protected:
	char m_buffer[256];

public:
	DerivableType* m_type;
	size_t m_offset;
	sl::Array<int32_t> m_llvmIndexArray;
	size_t m_vtableIndex;

public:
	BaseTypeCoord();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// unfortunately, LLVM does not natively support unions
// therefore, unnamed unions on the way to a member need special handling

struct UnionCoord {
	UnionType* m_type;
	intptr_t m_level; // signed for simplier comparisons
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class MemberCoord: public BaseTypeCoord {
protected:
	char m_buffer[256];

public:
	sl::Array<UnionCoord> m_unionCoordArray;

	MemberCoord():
		m_unionCoordArray(rc::BufKind_Field, m_buffer, sizeof(m_buffer)) {}
};

//..............................................................................

class DerivableType:
	public NamedType,
	public MemberBlock {
	friend class Parser;

protected:
	class DefaultStaticConstructor: public CompilableFunction {
	public:
		DefaultStaticConstructor() {
			m_functionKind = FunctionKind_StaticConstructor;
			m_storageKind = StorageKind_Static;
		}

		virtual
		bool
		compile() {
			return ((DerivableType*)m_parentNamespace)->compileDefaultStaticConstructor();
		}
	};

	class DefaultConstructor: public CompilableFunction {
	public:
		DefaultConstructor() {
			m_functionKind = FunctionKind_Constructor;
		}

		virtual
		bool
		compile() {
			return ((DerivableType*)m_parentNamespace)->compileDefaultConstructor();
		}
	};

	class DefaultDestructor: public CompilableFunction {
	public:
		DefaultDestructor() {
			m_functionKind = FunctionKind_Destructor;
		}

		virtual
		bool
		compile() {
			return ((DerivableType*)m_parentNamespace)->compileDefaultDestructor();
		}
	};

protected:
	// base types

	sl::StringHashTable<BaseTypeSlot*> m_baseTypeMap;
	sl::List<BaseTypeSlot> m_baseTypeList;
	sl::Array<BaseTypeSlot*> m_baseTypeArray;
	sl::Array<BaseTypeSlot*> m_gcRootBaseTypeArray;
	sl::Array<BaseTypeSlot*> m_baseTypeConstructArray;
	sl::Array<BaseTypeSlot*> m_baseTypeDestructArray;

	Type* m_setAsType;

	// overloaded operators

	sl::Array<OverloadableFunction> m_unaryOperatorTable;
	sl::Array<OverloadableFunction> m_binaryOperatorTable;
	sl::Array<Function*> m_castOperatorArray;
	sl::StringHashTable<Function*> m_castOperatorMap;
	OverloadableFunction m_callOperator;
	Function* m_operatorVararg;
	Function* m_operatorCdeclVararg;
	sl::StringHashTable<Property*> m_indexerPropertyMap;

public:
	DerivableType();

	virtual
	Type*
	getThisArgType(uint_t ptrTypeFlags) {
		return (Type*)getDataPtrType(DataPtrTypeKind_Normal, ptrTypeFlags);
	}

	FunctionType*
	getMemberMethodType(
		FunctionType* shortType,
		uint_t thisArgTypeFlags = 0
	);

	PropertyType*
	getMemberPropertyType(PropertyType* shortType);

	sl::ConstList<BaseTypeSlot>
	getBaseTypeList() {
		return m_baseTypeList;
	}

	const sl::Array<BaseTypeSlot*>&
	getBaseTypeArray() {
		return m_baseTypeArray;
	}

	BaseTypeSlot*
	getBaseTypeByIndex(size_t index);

	BaseTypeSlot*
	addBaseType(Type* type);

	BaseTypeSlot*
	findBaseType(Type* type) {
		sl::StringHashTableIterator<BaseTypeSlot*> it = m_baseTypeMap.find(type->getSignature());
		return it ? it->m_value : NULL;
	}

	bool
	findBaseTypeTraverse(
		Type* type,
		BaseTypeCoord* coord = NULL
	) {
		return findBaseTypeTraverseImpl(type, coord, 0);
	}

	size_t
	findBaseTypeOffset(Type* type);

	bool
	isDerivedType(Type* baseType) {
		return cmp(baseType) == 0 || findBaseTypeTraverse(baseType);
	}

	sl::Array<BaseTypeSlot*>
	getGcRootBaseTypeArray() {
		return m_gcRootBaseTypeArray;
	}

	Type*
	getSetAsType() {
		return m_setAsType;
	}

	OverloadableFunction
	getUnaryOperator(UnOpKind opKind) {
		return (size_t)opKind < m_unaryOperatorTable.getCount() ? m_unaryOperatorTable[opKind] : OverloadableFunction();
	}

	OverloadableFunction
	getBinaryOperator(BinOpKind opKind) {
		return (size_t)opKind < m_binaryOperatorTable.getCount() ? m_binaryOperatorTable[opKind] : OverloadableFunction();
	}

	const sl::Array<Function*>&
	getCastOperatorArray() {
		return m_castOperatorArray;
	}

	Function*
	getCastOperator(Type* type) {
		sl::StringHashTableIterator<Function*> it = m_castOperatorMap.find(type->getSignature());
		return it ? it->m_value : NULL;
	}

	OverloadableFunction
	getCallOperator() {
		return m_callOperator;
	}

	Function*
	getOperatorVararg() {
		return m_operatorVararg;
	}

	Function*
	getOperatorCdeclVararg() {
		return m_operatorCdeclVararg;
	}

	bool
	hasIndexerProperties() {
		return !m_indexerPropertyMap.isEmpty();
	}

	Property*
	chooseIndexerProperty(const Value& opValue);

	Field*
	getFieldByIndex(size_t index);

	virtual
	bool
	addMethod(Function* function);

	virtual
	bool
	addProperty(Property* prop);

	bool
	callBaseTypeConstructors(const Value& thisValue);

	bool
	callBaseTypeDestructors(const Value& thisValue);

	virtual
	bool
	require() {
		return ensureLayout() && requireConstructor();
	}

	virtual
	bool
	requireExternalReturn() {
		return DerivableType::require();
	}

	virtual
	sl::StringRef
	getValueString(
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
	) {
		return findDirectChildItemTraverse(name, coord, flags, 0);
	}

protected:
	virtual
	bool
	parseBody();

	virtual
	bool
	resolveImports();

	Property*
	getIndexerProperty(Type* argType);

	FindModuleItemResult
	findItemInExtensionNamespaces(const sl::StringRef& name);

	bool
	compileDefaultStaticConstructor();

	bool
	compileDefaultConstructor();

	bool
	compileDefaultDestructor();

	bool
	requireConstructor();

	bool
	findBaseTypeTraverseImpl(
		Type* type,
		BaseTypeCoord* coord,
		size_t level
	);

	FindModuleItemResult
	findDirectChildItemTraverse(
		const sl::StringRef& name,
		MemberCoord* coord,
		uint_t flags,
		size_t baseTypeLevel
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
