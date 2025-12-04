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

Properties
==========

In the context of a programming language, property is an entity that looks like a variable/field but allows performing actions on read or write. Functions implementing these actions are called **accessors**. The read accessor is called a **getter**, and the write accessor is called a **setter**. Each property has a single getter and optionally one or more setters.

If a setter is overloaded then the selection of particular setter function will be performed according to the same rules that apply to regular overloaded functions. If a property has no setters then it is a **const** property (**read-only** term has a special meaning in Jancy; more on that in **Dual Modifiers**).

Jancy provides two methods of declaring a property: simple and full.

.. toctree::
	:titlesonly:

	prop_simple.rst
	prop_full.rst
	prop_indexed.rst
	prop_autoget.rst
	prop_bindable.rst
