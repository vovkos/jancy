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

Dual Modifiers
==============

The namespace member access control model of Jancy differs from that of most object-oriented languages.

* There are only two access specifiers:
	* ``public``
	* ``protected``
* Member access can be specified in two styles:
	* C++-style (i.e. a label)
	* Java-style (i.e. a declaration specifier)
* The default access specifier is public -- even for classes
* Global namespace members can also have access specifiers just like named type members (and why not?)

The main difference, of course, is the first one. For each namespace the rest of the namespaces falls into one of the two categories:

* Friends
	* This namespace
	* Nested namespaces
	* Namespaces of derived types
	* Extension namespaces
	* Namespaces declared as **friends**
* Aliens
	* Everybody else

Friends have access to all the members including protected ones. Aliens can only access public members.

Admittedly, this approach definitely affords the developer a much lesser degree of flexibility in fine-tuning of who-can-access-what (other object oriented languages typically provide from three to five access specifiers).

On the positive side, this simplified binary model of friends vs aliens and opens up the possibility of dual modifiers, that is, the modifiers having one meaning for friends and another for aliens.

readonly
--------

One of the most common elements used in virtually every program is a read-only field. A class sets and modifies a field; all users of the class can only read this field.

Conventionally this is implemented by declaring a private field and a public getter.

The implementation relying on dual modifiers looks a lot more natural, as it is compact -- Jancy's dual modifier ``readonly`` is ignored by friends and means ``const`` for aliens:

.. code-block:: jnc

	class C1
	{
	    int readonly mreadOnly;

	    foo ()
	    {
	        mreadOnly = 10; // for insiders it's a regular field
	    }
	}

	bar ()
	{
	    C1 c;
	    int x = c.mreadOnly; // no problem
	    c.mreadOnly = 20;    // error: cannot assign to const-location
	}

event
-----

Events represent yet another extremely common programming element that requires dual access control.

The owner of an event must have the full control over this event, including the possibility of actually firing it. Subscribers are only able to add and remove event handlers.

The dual modifier ``event`` provides full multicast-access to friends and event-only access to aliens:

.. code-block:: jnc

	class C1
	{
	    event m_onCompleted ();

	    work ()
	    {
	        // ...

	        m_onCompleted (); // insiders have multicast-access to m_onCompleted
	    }
	}

	onCompleted ()
	{
	    // ...
	}

	foo ()
	{
	    C1 c;
	    c.m_onCompleted += onCompleted; // aliens have event-access to m_onCompleted
	    c.m_completeEvent (); // error: aliens have no multicast-access to m_onCompleted
	}
