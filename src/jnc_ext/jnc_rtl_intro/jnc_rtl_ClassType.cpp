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
#include "jnc_rtl_ClassType.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_FunctionOverload.h"
#include "jnc_ct_Module.h"
#include "jnc_Construct.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ClassType,
	"jnc.ClassType",
	sl::g_nullGuid,
	-1,
	ClassType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(ClassType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<ClassType, ct::ClassType*>))
	JNC_MAP_CONST_PROPERTY("m_classTypeKind", &ClassType::getClassTypeKind)
	JNC_MAP_CONST_PROPERTY("m_ifaceStructType", &ClassType::getIfaceStructType)
	JNC_MAP_CONST_PROPERTY("m_classStructType", &ClassType::getClassStructType)
	JNC_MAP_FUNCTION("getClassPtrType", &ClassType::getClassPtrType)
	JNC_MAP_FUNCTION("createObject", &ClassType::createObject)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ClassPtrType,
	"jnc.ClassPtrType",
	sl::g_nullGuid,
	-1,
	ClassPtrType,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(ClassPtrType)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<ClassPtrType, ct::ClassPtrType*>))
	JNC_MAP_CONST_PROPERTY("m_ptrTypeKind", &ClassPtrType::getPtrTypeKind)
	JNC_MAP_CONST_PROPERTY("m_targetType", &ClassPtrType::getTargetType)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

IfaceHdr*
ClassType::createObject() {
	if (m_item->getFlags() & (ClassTypeFlag_HasAbstractMethods | ClassTypeFlag_OpaqueNonCreatable)) {
		err::setFormatStringError("cannot instantiate '%s'", m_item->getTypeString().sz());
		return NULL;
	}

	IfaceHdr* p = jnc::rt::getCurrentThreadRuntime()->getGcHeap()->allocateClass(m_item);
	OverloadableFunction constructor = m_item->getConstructor();
	if (!constructor)
		return p;

	ct::Function* simpleConstructor = NULL;
	if (constructor->getItemKind() == jnc_ModuleItemKind_Function) {
		if (constructor.getFunction()->getType()->getShortType()->getArgArray().isEmpty())
			simpleConstructor = constructor.getFunction();
	} else {
		ct::FunctionOverload* overloadedConstructor = constructor.getFunctionOverload();
		size_t count = overloadedConstructor->getOverloadCount();
		for (size_t i = 0; i < count; i++) {
			ct::Function* overload = overloadedConstructor->getOverload(i);
			if (overload->getType()->getShortType()->getArgArray().isEmpty()) {
				simpleConstructor = overload;
				break;
			}
		}
	}

	if (!simpleConstructor) {
		err::setError("cannot dynamically instantiate classes with non-trivial constructors");
		return NULL;
	}

	callVoidFunction(simpleConstructor, p);
	return p;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
