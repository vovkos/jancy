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

if(WIN32)
	configure_file(
		sign-jncx.bat.in
		sign-jncx.bat
		)

	configure_file(
		sign-all-jncx.bat.in
		sign-all-jncx.bat
		)
else()
	set(DOLLAR $)

	axl_configure_file_w_permissions(
		sign-jncx.sh.in
		sign-jncx.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)

	axl_configure_file_w_permissions(
		sign-all-jncx.sh.in
		sign-all-jncx.sh
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_WRITE WORLD_EXECUTE
		)
endif()

#...............................................................................
