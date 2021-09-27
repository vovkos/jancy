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
#include "jnc_ct_MemberBlock.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(MemberBlock)

//..............................................................................

class MemberBlock: public IfaceHdr {
protected:
	ct::MemberBlock* m_block;

public:
	MemberBlock(ct::MemberBlock* block) {
		m_block = block;
	}

public:
	Function*
	JNC_CDECL
	getStaticConstructor() {
		return rtl::getFunction(m_block->getStaticConstructor());
	}

	Function*
	JNC_CDECL
	getConstructor() {
		return rtl::getFunction(m_block->getConstructor());
	}

	Function*
	JNC_CDECL
	getDestructor() {
		return rtl::getFunction(m_block->getDestructor());
	}

	size_t
	JNC_CDECL
	getStaticVariableCount() {
		return m_block->getStaticVariableArray().getCount();
	}

	Variable*
	JNC_CDECL
	getStaticVariable(size_t index) {
		size_t count = m_block->getStaticVariableArray().getCount();
		return index < count ? rtl::getVariable(m_block->getStaticVariableArray()[index]) : NULL;
	}

	size_t
	JNC_CDECL
	getFieldCount() {
		return m_block->getFieldArray().getCount();
	}

	Field*
	JNC_CDECL
	getField(size_t index) {
		size_t count = m_block->getFieldArray().getCount();
		return index < count ? rtl::getField(m_block->getFieldArray()[index]) : NULL;
	}

	size_t
	JNC_CDECL
	getMethodCount() {
		return m_block->getMethodArray().getCount();
	}

	Function*
	JNC_CDECL
	getMethod(size_t index) {
		size_t count = m_block->getMethodArray().getCount();
		return index < count ? rtl::getFunction(m_block->getMethodArray()[index]) : NULL;
	}

	size_t
	JNC_CDECL
	getPropertyCount() {
		return m_block->getPropertyArray().getCount();
	}

	Property*
	JNC_CDECL
	getProperty(size_t index) {
		size_t count = m_block->getPropertyArray().getCount();
		return index < count ? rtl::getProperty(m_block->getPropertyArray()[index]) : NULL;
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
