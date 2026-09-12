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

Function Redirection
====================

In Jancy it's possible to redirect functions (to share implementation) using initializer syntax. This feature is somewhat reminiscent of `aliases <aliases>`_ (both redirect), but not quite the same semantically.

Alias is just a name which maps (redirects) to another name. Consider overloaded function. What if we want to redirect a particular overload? Or what if we want to redirect an unnamed function, such as a destructor or a getter? This obviously can't be done with aliases.

Function redirection solves exactly this problem. Declare a function (it may be overloaded, be a special method like ``destruct``, ``get``, etc) and then write an initializers for it. Initializer is what this function will be redirected to.

.. code-block:: jnc

	class Class {
		destruct() = close;

		close();

		void fn() = implementation0;
		void fn(int) = implementation1;
		void fn(double) = implementation2;
	}
