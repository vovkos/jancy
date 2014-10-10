while (chomp (my $s = <>))
{
	if ($s =~ m/\/\/\//)
	{
		# inject /// comments directly as C++ snippets 

		print ("$'");
	}
	else 
	{
		# get rid of single-line comments

		$s =~ s/\/\/.*//;

		# if not an empty line or all-space line, make it C-literal

		if ($s !~ m/^[ \t]*$/)
		{
			print ("\"$s\\n\"");
		}	
	}	

	print ("\n");
}