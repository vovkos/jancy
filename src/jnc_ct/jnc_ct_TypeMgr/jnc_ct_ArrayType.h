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

#include "jnc_ct_Type.h"
#include "jnc_ct_Function.h"
#include "jnc_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

class ArrayType: public Type {
	friend class TypeMgr;
	friend class Parser;

protected:
	class GetDynamicSizeFunction: public CompilableFunction {
	public:
		ArrayType* m_arrayType;

	public:
		virtual
		bool
		compile() {
			return m_arrayType->compileGetDynamicSizeFunction(this);
		}
	};

protected:
	Type* m_elementType;
	Type* m_rootType;
	size_t m_elementCount;

	sl::List<Token> m_elementCountInitializer;
	GetDynamicSizeFunction* m_getDynamicSizeFunction;

	Unit* m_parentUnit;
	Namespace* m_parentNamespace;

public:
	ArrayType();

	Type*
	getElementType() {
		return m_elementType;
	}

	Type*
	getRootType();

	size_t
	getElementCount() {
		return m_elementCount;
	}

	sl::List<Token>*
	getElementCountInitializer() {
		return &m_elementCountInitializer;
	}

	Function* getGetDynamicSizeFunction() {
		return m_getDynamicSizeFunction;
	}

	static
	sl::String
	createSignature(
		Type* elementType,
		size_t elementCount
	) {
		return sl::formatString(
			"A%d%s",
			elementCount,
			elementType->getSignature().sz()
		);
	}

	virtual
	sl::StringRef
	getValueString(
		const void* p,
		const char* formatSpec
	);

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	);

	bool
	ensureDynamicLayout(
		StructType* dynamicStruct,
		Field* dynamicField
	);

protected:
	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_elementType, m_elementCount);
	}

	virtual
	bool
	resolveImports() {
		return m_elementType->ensureNoImports();
	}

	virtual
	bool
	calcLayout() {
		return calcLayoutImpl(NULL, NULL);
	}

	bool
	calcLayoutImpl(
		StructType* dynamicStruct,
		Field* dynamicField
	);

	virtual
	void
	prepareTypeString();

	virtual
	void
	prepareDoxyLinkedText();

	sl::String
	createDimensionString();

	virtual
	void
	prepareLlvmType() {
		ASSERT(m_elementCount != -1);
		m_llvmType = llvm::ArrayType::get(m_elementType->getLlvmType(), m_elementCount);
	}

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_ArrayType);
	}

	bool
	compileGetDynamicSizeFunction(Function* function);
};

//..............................................................................

} // namespace ct
} // namespace jnc
