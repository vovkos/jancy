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

break/continue
==============

Jancy features multi-level loop jumps. These are achieved with **break-n** and **continue-n**:

.. code-block:: jnc

	int a [3] [4] =
	{
	    { 1,  2,  3,  4 },
	    { 5,  6, -7,  8 },
	    { 9, 10, 11, 12 },
	};

	for (size_t i = 0; i < countof (a); i++)
	    for (size_t j = 0; j < countof (a [0]); j++)
	        if (a [i] [j] < 0)
	        {
	            // negative element is found, process it...

	            break2; // exit 2 loops at once
	        }
