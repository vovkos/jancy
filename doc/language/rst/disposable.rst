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

Deterministic Resource Release
==============================

Jancy provides an extremely convenient facility for ensuring the deterministic resource release in an inherently non-deterministic GC-world. This is achieved with ``disposable`` storage specifier.
