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
#include "jnc_ct_ThunkFunction.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
ThunkFunction::compile() {
	ASSERT(m_targetFunction);

	bool result;

	sl::Array<FunctionArg*> argArray = m_type->getArgArray();
	size_t argCount = argArray.getCount();

	char buffer[256];
	sl::Array<Value> thunkArgValueArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	thunkArgValueArray.setCount(argCount);

	m_module->m_functionMgr.internalPrologue(this, thunkArgValueArray.p(), argCount);

	sl::Array<FunctionArg*> targetArgArray = m_targetFunction->getType()->getArgArray();
	size_t targetArgCount = targetArgArray.getCount();

	// skip the fat -> thin thunk 1st (closure-this) argument

	size_t j =
		(argCount && argArray[0]->getStorageKind() == StorageKind_This) &&
		(!targetArgCount || targetArgArray[0]->getStorageKind() != StorageKind_This) ? 1 : 0;

	if (argCount > j + targetArgCount)
		argCount = j + targetArgCount; // trim unused arguments

	sl::BoxList<Value> targetArgValueList;
	for (size_t i = 0; j < argCount; i++, j++) {
		Value* argValue = targetArgValueList.insertTail().p();

		result = m_module->m_operatorMgr.castOperator(
			thunkArgValueArray[j],
			targetArgArray[i]->getType(),
			argValue
		);

		if (!result)
			return false;
	}

	Value returnValue;
	result = m_module->m_operatorMgr.callOperator(
		m_targetFunction,
		&targetArgValueList,
		&returnValue
	);

	if (m_type->getReturnType()->getTypeKind() != TypeKind_Void) {
		result = m_module->m_controlFlowMgr.ret(returnValue);
		if (!result)
			return false;
	}

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
