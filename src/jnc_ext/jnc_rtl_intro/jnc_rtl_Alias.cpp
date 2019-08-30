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
#include "jnc_rtl_Alias.h"
#include "jnc_Construct.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Alias,
	"jnc.Alias",
	sl::g_nullGuid,
	-1,
	Alias,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(Alias)
	JNC_MAP_CONSTRUCTOR((&jnc::construct<Alias, ct::Alias*>))
	JNC_MAP_CONST_PROPERTY("m_targetItem", &Alias::getTargetItem)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace rtl
} // namespace jnc
