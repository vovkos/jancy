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

.. _aliases:

Aliases
=======

Aliases provide a convenient facility for accessing items via alternative names.

Sometimes, this can handy for shortening long qualified identifiers:

.. code-block:: jnc

	alias p = the.longest.qualified.identifier.ever;

But more importantly, it allows creating bindings between *special items*, such as *bindable events* or ``dispose`` methods:

.. code-block:: jnc

	opaque class EnumProperty: Property {
		property m_value {
			variant_t autoget m_value;
			set(variant_t value);
			bindable alias m_onPropChanged = m_onChanged;
		}

		property m_currentIndex {
			size_t autoget m_value;
			set(size_t value);
			bindable alias m_onPropChanged = m_onChanged;
		}

		event m_onChanged();

		// ...
	}

	alias p = very.long.qualified.identifier;

In the example above, two properties (``m_value`` and ``m_currentIndex``) share the same event ``m_onChanged`` as bindable event (i.e. the one to be used for notification of property change).

In the next example, we use the natural method name ``close`` and then define an alias which effectively makes the whole class *disposable*:

.. code-block:: jnc

	opaque class File {
		// ...

		void close();

		alias dispose = close;
	}
