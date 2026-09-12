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

Data Pointers
=============

Do we even need data pointers? In C/C++ world that's not much of a question: using pointers is the one and **only way** of working with **dynamic memory**. What about the **managed world**, do we need data pointers there?

**Most** managed language designers believe that the answer is *NO*. This is largely because data pointers fall into the disadvantageous area on the risk/reward chart for most programming tasks. There is, however, one programming area where the use of data pointers **truly shines**: working with **binary data**.

Here is an example. Try to write Java code that deconstructs a TCP/IP packet. Compare the resulting **mess** of fixed index array references and word types assembled from bytes to the **clean and efficient** code in C that will **superimpose** protocol **header structs** on the buffer data, then direct access **struct fields** abd traverse the packet using **pointer arithmetic**!

Since Jancy was conceived as the scripting language for our [IO Ninja](/ioninja/) software, living without pointers was out of the question. Instead, we made **data pointers safe**. Safe data pointers and safe pointer arithmetic are among the **biggest innovations** of Jancy.

Jancy pointers are **fat** by default and indirectlty contain information about the **allowed range**. Access to data via a pointer that failed the **range check** is not allowed:

.. code-block:: jnc

	void foo(size_t i) {
		int a[] = { 1, 2, 3 };
		//...

		int* p = a;
		p += i;
		*p = 40;    // <-- error: out-of-bounds
		a[i] = 40; // <-- error: out-of-bounds
	}

	void bar() {
		//...
		foo(3);
	}

Range is checked on both **array accesses** and **pointer dereferences**.

Unfortunately, **range checks are not enough**, there are other pointer-related issues. But the good thing is, these are covered, too.

**Dangling pointers** are impossible in Jancy. First of all, Jancy uses garbage collection (GC) as a memory management mechanism, and with GC memory block gets freed only when there are no more pointers referencing it.

What about locals, you might ask? Any local taken "fat" address of, is being **lifted to GC** heap.

.. code-block:: jnc

	int* foo() {
		int a = 10;
		// ...

		return &a; // <-- no problem
	}

There are **no uninitialzed pointers**, either -- Jancy compiler zeros every variable before any user code can touch it.

There is also a danger of **corrupting** or replacing the information used for validation, and below you can observe a couple of ways of doing so. Well, Jancy **prevents that**, too -- either at compile time, or at run time.

.. code-block:: jnc

	struct PodParent {
		int m_a;
	}

	struct NonPodChild: PodParent {
		char const* m_s;
	}

	union IllegalUnion {
		intptr_t m_a[2];
		void* m_p; // <-- error
	}

	void upcast(NonPodChild* b) {
		PodParent* a = b;
	}

	void reinterpretCast(PodParent* a, NonPodChild* b) {
		char* p = (char*) a;  // <-- OK
		char* p2 = (char*) b; // <-- error
		char const* p3 = (char const*) b; // <-- OK
	}

	void downcast(PodParent* a) {
		NonPodChild* c = (NonPodChild*) a; // <-- error
		NonPodChild const* c = (NonPodChild const*) a; // <-- error
		NonPodChild* c = dynamic (NonPodChild*) a; // OK
	}

Besides the normal (fat) data **pointer with validators** Jancy also supports ``thin`` data pointers, which only hold the **target address**. This might be useful when writing **performance-critical** code, or for **interoperability** with the external C libraries. ``thin`` pointers are **not safe**, obviously -- they simply lack the information which can be used for safety checks.
