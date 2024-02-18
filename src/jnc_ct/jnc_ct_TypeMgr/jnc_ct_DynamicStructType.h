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

namespace jnc {
namespace ct {

//..............................................................................

class DynamicStructType: public NamedType {
	friend class TypeMgr;

protected:
	size_t m_fieldAlignment;

public:
	DynamicStructType();

	size_t
	getFieldAlignment() {
		return m_fieldAlignment;
	}

protected:
	virtual
	void
	prepareSignature() {
		m_signature = 'D' + m_qualifiedName;
		m_flags |= TypeFlag_SignatureFinal;
	}

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_DynamicStructType);
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
