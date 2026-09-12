.. .............................................................................
..
..  This file is part of the Jancy toolkit.
..
..  Jancy is distributed under the MIT license.
..  For details see accompanying license.txt file,
..  the public copy of which is also available at:
..  http://tibbo.com/downloads/archive/jancy/license.txt
..
.. .............................................................................

Literals
========

Jancy has the following kinds of literals:

* C-literal;
* Binary literals:

	* Hex-radix literal;
	* Oct-radix literal;
	* Bin-radix literal;
	* Dec-radix literal;

* Formatting literal.

The first one is the good old C-style literal. It defines a statically allocated const char array.

.. code-block:: jnc

	char a[] = "hello world";

The second kind is the binary literal. This kind of literals allows for a nice and clean way of defining in-program const binary data blocks (i.e. icons, public keys etc) Just like C-literals, hex literals define a statically allocated const char array.

.. code-block:: jnc

	char b[] = 0x"61 62 63 20 64 65 66 00";
	// same as: char b[] = { 0x61, 0x62, 0x63, 0x20, 0x64, 0x65, 0x66, 0x00  }

It's OK to use upper-case or lower-case and group hex codes with spaces to your liking (or not use spaces for grouping at all):

.. code-block:: jnc

	char c[] = 0x"696a 6b6c 6D6E 6F70 0000";

Concatenating hex and C-literals can be used for removing trailing zeroes from C-literals thus producing non-zero-terminated literals:

.. code-block:: jnc

	char d[] = "non-zero-terminated" 0x"";

Both C-literals and binary literals with any radix can be multi-line:

.. code-block:: jnc

	char m[] = """
		This is a multi-line literal. Note that Jancy will auto-align it.
		Thus, there is no need to create an ugly de-indentation structure
		required in most other languages with multi-line literals.
		""";

The last kind of Jancy literals is the formatting literal. Literals of this kind bring Perl-style formatting into our C-family language. A formatting literal produces a dynamically allocated char array on the GC heap:

.. code-block:: jnc

	int i = 100;

	char const* c = $"i = $i";

Jancy allows the use of expressions and printf-style formatting:

.. code-block:: jnc

	int i = 100;
	char a[] = "hello world";
	uint_t h = 0xbeef;

	char const* c = $"i = $i; a[6] = $(a[6]; c); h = 0x$(h; 08x)";

It's also OK to specify some of injected values (or all of them) in the argument list of the formatting literal and reference these values by index. You can even reference the same value multiple times to display it using different format specifiers:

.. code-block:: jnc

	char const* c = $"rgb dec = (%1, %2, %3); rgb hex = (%(1;x), %(2;x), %(3;x))" (
		(colorTable[i].m_value & 0xff0000) >> 16,
		(colorTable[i].m_value & 0x00ff00) >> 8,
		colorTable[i].m_value & 0x0000ff
	);

Last but not least, all literal kinds can be concatenated and combined. If the combination does not include formatting literals, then the result is a statically allocated const char array. If the combination includes formatting literals then it will produce a dynamically allocated char array on the GC heap:

.. code-block:: jnc

	int i = 100;
	char a[] = "hello world";

	char const* c =
		0x"61 62 63"
		" ...concatenated to... "
		$"i = $i; a = $a; "
		0x"64 65 66"
		" ...end."
		);
