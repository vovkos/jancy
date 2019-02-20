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

# 0 -- idle
# 1 -- converting to C-literal
# 2 -- C-comment

$state = 0;

while (chomp (my $s = <>))
{
	if ($state == 2)
	{
		if ($s =~ m/\*\//)
		{
			$s = $';
			$state = 1; # we only handle C-comments in state 1
		}
		else
		{
			next;
		}
	}

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
	elsif ($state == 1)
	{
		# get rid of comments (don't bother handling multiple /**/ per line)

		if ($s =~ m/\/[\/*]/)
		{
			if ($& eq "/*")
			{
				$state = 2;
				next;
			}
			else
			{
				$s = $`;
			}
		}

		# if not an empty line or all-space line, make it C-literal

		if ($s !~ m/^[ \t]*$/)
		{
			# remove trailing whitespace

			$s =~ s/\s+$//;

			# mask quotation marks with escapes

			$s =~ s/(?<!\\)\"/\\\"/g;

			print ("\"$s\\n\"\n");
		}
	}
}
