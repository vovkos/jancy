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

Reactive Programming
====================

Jancy is one of the few imperative languages with the support for reactive programming.

Reactive programming is something that anyone who ever used Excel is intuitively familiar with. Writing a “formula” in cell A that references cell B creates a **dependency**. Change the value in B, and the cell A will get updated, too. There is no need to write an event handler to be invoked on every update in cell B -- all changes are tracked automatically.

Things are not so easy in common programming languages. UI widgets provide events that fire when certain properties change, and if you need to track these changes and do something in response then you write an event handler, subscribe to an event, and update dependent controls/value from within the handler.

Jancy brings the Excel-like automatic execution of a “formula” when values referred to by that formula change. Write a relation between two or more UI properties, and the updates will happen automatically.

.. code-block:: jnc

	// ...
	m_isTransmitEnabled = m_state == State.Connected;
	m_actionTable [ActionId.Disconnect].m_isEnabled = m_state != State.Closed;
	// ...

How does Jancy know where to use Excel-like execution and where to use the traditional imperative approach?

Reactors.

You declare dedicated sections of reactive code, or so-called reactors. Expressions within reactors behave like formulas in Excel and get automatically re-evaluated when bindable properties referred by the given expression change. All the dependency building, subscribing, and unsubscribing happens automatically behind the scenes.

.. code-block:: jnc

	reactor TcpConnectionSession.m_uiReactor ()
	{
	    m_title = $"TCP $(m_addressCombo.m_editText)";
	    m_isTransmitEnabled = m_state == State.Connected;
	    m_actionTable [ActionId.Disconnect].m_isEnabled = m_state != State.Closed;
	    m_adapterProp.m_isEnabled = m_useLocalAddressProp.m_value;
	    m_localPortProp.m_isEnabled = m_useLocalAddressProp.m_value;
	}

Reactors specify the boundaries of where to use the reactive approach. In addition you are also in full control of when to use it because reactors can be started and stopped as needed.

.. code-block:: jnc

	TcpConnectionSession.construct ()
	{
	    // ...
	    m_uiReactor.start ();
	}

Sometimes, expressions don't quite cut it when it comes to describing what has to be done in response to a property change: e.g. running a cycle, or executing a sequence of statements. Using expressions in **reactor** blocks might not provide enough control over which actions must be taken in response to what property change.

The **onevent** declaration in **reactor** blocks gives you fine-grained control over dependencies and at the same time frees you from manually binding/unbinding to/from events:

.. code-block:: jnc

	reactor g_myReactor ()
	{
	    onevent bindingof (g_state) ()
	    {
	        // handle state change
	    }

	    // onevent statement allows binding to any events, not just to 'onChanged'

	    onevent g_onApplyIpSettings ()
	    {
	        // apply IP settings...
	    }
	}

All in all, reactors simplify UI programming by an order of magnitude.
