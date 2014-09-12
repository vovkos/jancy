set (VERSION_MAJOR     1)
set (VERSION_MINOR     0)
set (VERSION_REVISION  0)
set (VERSION_TAG)

if (NOT "${VERSION_TAG}" STREQUAL "")
	set (VERSION_TAG_SUFFIX " ${VERSION_TAG}")
else ()
	set (VERSION_TAG_SUFFIX)
endif ()

set (VERSION_COMPANY   "Tibbo Technology Inc")
set (VERSION_YEARS     "2007-2014")
