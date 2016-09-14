Global Namespaces
=================

Just like in C++ and C\#, Jancy features support for global namespaces. It is OK to open a nested namespace in one go; it is also OK to open and close the same namespace multiple times for adding new elements.

.. code-block:: none

	namespace a.b.c {
	namespace d {

	} // namespace a.b.c.d
	} // namespace a.b.c

	namespace a.b.c.d {
	} // namespace a.b.c.d

