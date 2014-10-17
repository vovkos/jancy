# This file is part of Bulldozer project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

configure_file (
    prepare-contents.in
    prepare-contents
    )

add_subdirectory (nj_deb_package)
add_subdirectory (nj_rpm_package)
add_subdirectory (nj_tar_archive)

#..............................................................................
