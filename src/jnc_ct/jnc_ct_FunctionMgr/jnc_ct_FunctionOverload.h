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

#include "jnc_ct_FunctionName.h"
#include "jnc_ct_FunctionTypeOverload.h"

namespace jnc {
namespace ct {

class Function;

//..............................................................................

class FunctionOverload:
	public ModuleItem,
	public ModuleItemDecl,
	public FunctionName {
	friend class FunctionMgr;

protected:
	FunctionTypeOverload m_typeOverload;
	sl::Array<Function*> m_overloadArray;

public:
	FunctionOverload() {
		m_itemKind = ModuleItemKind_FunctionOverload;
	}

	const FunctionTypeOverload&
	getTypeOverload() {
		return m_typeOverload;
	}

	size_t
	getOverloadCount() {
		return m_overloadArray.getCount();
	}

	Function*
	getOverload(size_t overloadIdx) {
		return m_overloadArray[overloadIdx];
	}

	Function*
	findOverload(FunctionType* type) {
		size_t i = m_typeOverload.findOverload(type);
		return i != -1 ? getOverload(i) : NULL;
	}

	Function*
	findShortOverload(FunctionType* type) {
		size_t i = m_typeOverload.findShortOverload(type);
		return i != -1 ? getOverload(i) : NULL;
	}

	Function*
	chooseOverload(
		Closure* closure,
		FunctionArg* const* argArray,
		size_t argCount,
		CastKind* castKind = NULL
	) {
		size_t i = m_typeOverload.chooseOverload(closure, argArray, argCount, castKind);
		return i != -1 ? getOverload(i) : NULL;
	}

	Function*
	chooseOverload(
		FunctionArg* const* argArray,
		size_t argCount,
		CastKind* castKind = NULL
	) {
		size_t i = m_typeOverload.chooseOverload(argArray, argCount, castKind);
		return i != -1 ? getOverload(i) : NULL;
	}

	Function*
	chooseOverload(
		Closure* closure,
		const sl::ArrayRef<FunctionArg*>& argArray,
		CastKind* castKind = NULL
	) {
		return chooseOverload(closure, argArray, argArray.getCount(), castKind);
	}

	Function*
	chooseOverload(
		const sl::ArrayRef<FunctionArg*>& argArray,
		CastKind* castKind = NULL
	) {
		return chooseOverload(argArray, argArray.getCount(), castKind);
	}

	Function*
	chooseOverload(
		const Value* argValueArray,
		size_t argCount,
		CastKind* castKind = NULL
	) {
		size_t i = m_typeOverload.chooseOverload(argValueArray, argCount, castKind);
		return i != -1 ? getOverload(i) : NULL;
	}

	Function*
	chooseOverload(
		const sl::ConstBoxList<Value>& argList,
		CastKind* castKind = NULL
	) {
		size_t i = m_typeOverload.chooseOverload(argList, castKind);
		return i != -1 ? getOverload(i) : NULL;
	}

	Function*
	chooseSetterOverload(
		Type* argType,
		CastKind* castKind = NULL
	) {
		size_t i = m_typeOverload.chooseSetterOverload(argType, castKind);
		return i != -1 ? getOverload(i) : NULL;
	}

	Function*
	chooseSetterOverload(
		const Value& argValue,
		CastKind* castKind = NULL
	) {
		size_t i = m_typeOverload.chooseSetterOverload(argValue, castKind);
		return i != -1 ? getOverload(i) : NULL;
	}

	Function*
	chooseSetterOverload(
		FunctionType* functionType,
		CastKind* castKind = NULL
	) {
		size_t i = m_typeOverload.chooseSetterOverload(functionType, castKind);
		return i != -1 ? getOverload(i) : NULL;
	}

	size_t
	addOverload(Function* function);

	virtual
	bool
	require();

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
