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

switch
======

Jancy encloses all the case blocks in switch statements into implicitly created scopes. This means you are free to create and use local variables in switch statements:

.. code-block:: jnc

	void foo(int x) {
		switch (x) {
		case 0:
			int i = 10;
			break;

		case 1:
			int i = 20; // no problem: we are in different scope

		case 2:
			int i = 30; // no problem even when we fall-through from previous case label
			break;

		default:
			int i = 40; // still ok. you've got the idea
		}
	}

Multi-level breaks can be applied to switch statement as well. In example below **break2** is used to break out of the switch statment and then out of the outer loop:

.. code-block:: jnc

	for (;;) {
		Request request = getNextRequest();
		switch (request) {
		case Request.Terminate:
			break2; // out of the loop

		case Request.Open:
			// ...
			break;

		case Request.Connect:
			// ...
			break;

		// ...
		}
	}
