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
	configure_file(
		sign-jncx.sh.in
		sign-jncx.sh
		)

	configure_file(
		sign-all-jncx.sh.in
		sign-all-jncx.sh
		)
endif()

#...............................................................................
