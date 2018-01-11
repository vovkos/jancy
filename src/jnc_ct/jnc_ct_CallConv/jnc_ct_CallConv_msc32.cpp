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
#include "jnc_ct_CallConv_msc32.h"
#include "jnc_ct_FunctionType.h"

namespace jnc {
namespace ct {

//..............................................................................

#if (_JNC_CPP_MSC && JNC_PTR_SIZE == 4)

AXL_TODO ("beware: structs with sizes between 16 and 24 are returned incorrectly (coercion must be implemented in XxxCallConv_msc32)")

#endif

//..............................................................................

} // namespace ct
} // namespace jnc
