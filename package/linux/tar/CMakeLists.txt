# This file is part of Bulldozer project
# Tibbo Technology Inc (C) 2004-2013. All rights reserved
# Author: Vladimir Gladkov

#..............................................................................

configure_file_w_permissions (
	build-tar.sh.in
	build-tar.sh
	OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_WRITE GROUP_EXECUTE
	WORLD_READ WORLD_WRITE WORLD_EXECUTE
	)

#..............................................................................
