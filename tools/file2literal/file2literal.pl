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

while (chomp(my $s = <>)) {
	$s =~ s/\\/\\\\/g;
	$s =~ s/"/\\"/g;
	print("\"$s\\n\"\n");
}
