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

#define _JNC_UNIONTYPE_H

#include "jnc_DerivableType.h"

/// \addtogroup union-type
/// @{

//..............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_UnionType: jnc_DerivableType
{
};

#endif // _JNC_CORE

//..............................................................................

/// @}
