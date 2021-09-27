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

#include "jnc_ExtensionLib.h"
#include "jnc_rtl_IntrospectionLib.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Module)
JNC_DECLARE_OPAQUE_CLASS_TYPE(Unit)

//..............................................................................

class Module: public IfaceHdr {
protected:
	ct::Module* m_module;

public:
	Module(ct::Module* module) {
		m_module = module;
	}

	uint_t
	JNC_CDECL
	getCompileFlags() {
		return m_module->getCompileFlags();
	}

	ModuleCompileState
	JNC_CDECL
	getCompileState() {
		return m_module->getCompileState();
	}

	GlobalNamespace*
	JNC_CDECL
	getGlobalNamespace() {
		return rtl::getGlobalNamespace(m_module->m_namespaceMgr.getGlobalNamespace());
	}

	Type*
	JNC_CDECL
	getPrimitiveType(TypeKind typeKind) {
		return rtl::getType(m_module->m_typeMgr.getPrimitiveType(typeKind));
	}
};

//..............................................................................

class Unit: public IfaceHdr {
protected:
	ct::Unit* m_unit;
	DataPtr m_filePathPtr;

public:
	Unit(ct::Unit* unit) {
		m_unit = unit;
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	DataPtr
	JNC_CDECL
	getFilePath(Unit* self);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
