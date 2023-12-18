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

Schedulers
==========

Jancy implements the concept of function pointer scheduling. When passing a function pointers as a callback of some sort (completion routine, event handler etc) you are free to assign it a user-defined “scheduler”. The purpose of this scheduler is to ensure the execution of your callback in the correct environment (i.e. a specific worker thread, from within a Windows Message handler, under lock/mutex, and so on).

The scheduler is a built-in interface of the Jancy compiler:

.. code-block:: jnc

	namespace jnc {

	class Scheduler
	{
	    abstract schedule (function* f ());
	}

	} // namespace jnc {

Note that even the ``schedule`` method accepts a pointer to a function with no arguments, and you can schedule functions with arbitrary argument list, as arguments will be captured in the closure object.

To assign a scheduler you use @ operator (at):

.. code-block:: jnc

	class WorkerThread: jnc.Scheduler
	{
	    override schedule (function* f ())
	    {
	        // enqueue f and signal worker thread event
	    }

	    workerThread ()
	    {
	        for (;;)
	        {
	            // wait for worker thread event

	            function* f () = getNextRequest ();
	            f ();
	        }
	    }
	}

	foo (int x);

	bar ()
	{
	    WorkerThread workerThread;

	    function* f (int) = foo @ workerThread; // create a scheduled pointer

	    (foo @ workerThread) (100); // or schedule now

	    // ...

	    f (200); // call through a scheduled pointer
	}

Below is a real-life example (from our IO Ninja software) of assigning a socket event handler (which gets fired from within the socket IO thread) and scheduling it to be called from the main UI thread:

.. code-block:: jnc

	TcpListenerSession.construct (doc.PluginHost* pluginHost)
	{
	    // ...

	    m_listenerSocket = new io.Socket ();
	    m_listenerSocket.m_onSocketEvent +=
	        onListenerSocketEvent @ pluginHost.m_mainThreadScheduler;

	    // ...
	}
