#..............................................................................

set (JANCY_VERSION_MAJOR     1)
set (JANCY_VERSION_MINOR     7)
set (JANCY_VERSION_REVISION  1)
set (JANCY_VERSION_TAG)

set (JANCY_VERSION_FILE_SUFFIX "${JANCY_VERSION_MAJOR}.${JANCY_VERSION_MINOR}.${JANCY_VERSION_REVISION}")

if (NOT "${JANCY_VERSION_TAG}" STREQUAL "")
	set (JANCY_VERSION_TAG_SUFFIX " ${JANCY_VERSION_TAG}")
	set (JANCY_VERSION_FILE_SUFFIX "${JANCY_VERSION_FILE_SUFFIX}-${JANCY_VERSION_TAG}")
else ()
	set (JANCY_VERSION_TAG_SUFFIX)
endif ()

string (TIMESTAMP JANCY_VERSION_YEAR  "%Y")
string (TIMESTAMP JANCY_VERSION_MONTH "%m")
string (TIMESTAMP JANCY_VERSION_DAY   "%d")

set (JANCY_VERSION_COMPANY   "Tibbo Technology Inc")
set (JANCY_VERSION_YEARS     "2012-${JANCY_VERSION_YEAR}")

#..............................................................................
