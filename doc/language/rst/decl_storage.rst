Storage Specifiers
==================

Both global and local variables can have ``static`` or ``threadlocal`` storage specifiers. ``static`` means that the memory for variable is allocated at the program start and it stays there until the program terminates. ``threadlocal`` on the other hand, means that variable is allocated from the thread local storage -- each thread in program has its own copy of such variable. To avoid unnecessary performance penalties during thread start, two limitations are imposed on ``threadlocal`` variables. The first limitation is: thread variables cannot have initializers; the second -- thread variables cannot be aggregate (e.g. class, struct, array etc).

Now, what if you **do** need an aggregate thead variable and/or a thread variable which needs to be initilized? The answer is simple -- use pointers:

.. code-block:: none

	threadlocal MyClass* threadClass;

	// ...

	if (!threadClass)
		threadClass = new MyClass (10, 20, 30);

There is one more storage class, which actually cannot be explicitly expressed with storage specifier -- ``local`` storage. The actual memory for local storage can come from **stack** or from **GC-heap**, but the most important thing about local storage is this: every time intruction pointer goes through a variable declaration, this will be a **new** copy of the local variable.

If storage specifier is omitted, then global variables get assigned ``static`` storage class and local variables -- ``local`` storage class.

Member fields of classes, structs and unions can have both ``static`` and ``threadlocal`` storage specifiers -- the meaning of these are identical to what we have just discussed. Member fields can also have ``mutable`` storage specifier, which means that this member field can be modified using ``const`` pointer to the parent class/struct/union -- just like in C/C++.

Now let's talk about functions. Global functions always have ``static`` storage class. Member functions, on the other hand, can have the following storage specifiers:

* ``static``
* ``virtual``
* ``abstract``
* ``override``

``static`` in respect to member functions mean that this function does not accept implict ``this`` argument. Therefore, static functions cannot access any non-static member fileds. ``virtual``, ``abstract`` and ``override`` specifiers allow programmer to create virtual functions -- i.e. the functions which can be overriden in derived classes. Note that neither structs nor unions are not allowed to have virtual functions, only **classes** do.
