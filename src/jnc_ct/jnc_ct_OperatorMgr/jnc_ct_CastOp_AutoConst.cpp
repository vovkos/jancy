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
#include "jnc_ct_CastOp_AutoConst.h"
#include "jnc_ct_AutoConstType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

CastKind
Cast_AutoConst::getCastKind(
	const Value& opValue,
	Type* type0
) {
	ASSERT(type0->getTypeKind() == TypeKind_AutoConst);
	AutoConstType* type = (AutoConstType*)type0;
	return m_module->m_operatorMgr.getCastKind(opValue, type->getOriginalType());
}

bool
Cast_AutoConst::cast(
	const Value& opValue,
	Type* type0,
	Value* resultValue
) {
	ASSERT(type0->getTypeKind() == TypeKind_AutoConst);
	AutoConstType* type = (AutoConstType*)type0;

	Type* originalType = ((AutoConstType*)type)->getOriginalType();
	bool result = m_module->m_operatorMgr.castOperator(opValue, originalType, resultValue);
	if (!result)
		return false;

	resultValue->overrideType(opValue, type);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
