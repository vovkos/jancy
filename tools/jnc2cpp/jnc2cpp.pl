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

$state = 0;

while (chomp (my $s = <>))
{
	if ($s =~ m/\/\/\//)
	{
		$body = $';

		if ($body =~ m/^\+\+\+/)
		{
			$state = 1;
		}
		elsif ($body =~ m/^\-\-\-/)
		{
			$state = 0;
		}
		else
		{
			# inject as C++ snippet

			print ("$body\n");
		}
	}
	elsif ($state)
	{
		# get rid of single-line comments

		$s =~ s/\/\/.*//;

		# if not an empty line or all-space line, make it C-literal

		if ($s !~ m/^[ \t]*$/)
		{
			# mask quotation marks with escapes

			$s =~ s/(?<!\\)\"/\\\"/g;

			print ("\"$s\\n\"\n");
		}
	}
}
