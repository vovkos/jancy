axl_chain_include (settings.cmake)

if (GCC)
	# LLVM is compiled without RTTI by default, so set default to "no-rtti"

	axl_override_setting_once (GCC_FLAG_CPP_RTTI "-fno-rtti")
endif ()
