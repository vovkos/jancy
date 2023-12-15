#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

axl_chain_include(settings.cmake)

if(GCC)
	# LLVM is compiled without RTTI by default, so set default to "no-rtti"
	axl_override_setting_once(GCC_FLAG_CPP_RTTI "-fno-rtti")
endif()
