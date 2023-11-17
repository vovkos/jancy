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
#include "jnc_rtl_Module.h"
#include "jnc_Construct.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Module,
	"jnc.Module",
	sl::g_nullGuid,
	-1,
	Module,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Module)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Module, ct::Module*>))
	JNC_MAP_CONST_PROPERTY("m_compileFlags", &Module::getCompileFlags)
	JNC_MAP_CONST_PROPERTY("m_compileState", &Module::getCompileState)
	JNC_MAP_CONST_PROPERTY("m_globalNamespace", &Module::getGlobalNamespace)
	JNC_MAP_CONST_PROPERTY("m_primitiveTypeTable", &Module::getPrimitiveType)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Unit,
	"jnc.Unit",
	sl::g_nullGuid,
	-1,
	Unit,
	&Unit::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Unit)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Unit, ct::Unit*>))
	JNC_MAP_CONST_PROPERTY("m_filePath", &Unit::getFilePath)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
Unit::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	gcHeap->markString(m_filePath);
}

String
JNC_CDECL
Unit::getFilePath(Unit* self) {
	if (!self->m_filePath.m_length)
		self->m_filePath = createForeignString(self->m_unit->getFilePath(), false);

	return self->m_filePath;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
