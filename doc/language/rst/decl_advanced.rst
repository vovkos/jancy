Advanced Declarations
=====================

Just like C/C++ (and even more so) Jancy allows declaring quite sophisticated entities in one go; therefore, declarations in Jancy can be rather complex, and both specifier and declarator can be composite.

Specifiers can contain:

* Access specifiers (``public`` or ``protected``)
* Storage specifiers (e.g. ``static``, ``typedef``, ``virtual`` etc)
* Type specifiers (e.g. ``char``, ``int``, ``double`` etc)
* Type modifiers (e.g. ``const``, ``volatile``, ``unsigned`` etc)

Declarators can contain:

* Type modifiers (e.g. ``const``, ``volatile``, ``unsigned`` etc)
* Pointer prefixes (``*``)
* Declarator identifiers (e.g. ``myVariable``)
* Declarator special function kind (e.g. ``construct``, ``get``, ``set`` etc)
* Array suffixes (e.g. ``[10]``)
* Function suffixes (e.g. ``(int, double)``)
* Bitfied suffixes (e.g. ``: 10``)
* Initializers (e.g. ``= 0``)
* Function bodies  (e.g. ``{ return 0; }``)

A realistic example of a complex declaration could look like:

.. code-block:: none

	static int const* function* a [2] () = { foo, bar };

Here ``static`` is a storage specifier, ``int`` is a type specifier, ``const`` and ``function`` are type modifiers, two ``*`` (asterisks) are pointer prefixes, ``a`` is a declarator name, ``[2]`` is a array suffix, ``()`` is a function suffix and ``= { foo, bar }`` is an initializer. This line declares a static variable ``a`` as an array of two elements of type "pointer to a function which takes no arguments and returns a const-pointer to ``int`` and initializes it with pointers to functions ``foo`` and ``bar``. Wheh! Seriously, it's much easier to read **the code itself** than its explanation.

C/C++ equivalent of the above example would look like:

.. code-block:: none

	static int const* (*a [2]) () = { foo, bar };

Now, if we add one extra layer of function pointers, C/C++ falls short of declaring it in one go (you will receive **function returns function** error); Jancy syntax still allows to do so (not like that could be crucial in any realistic scenario; just a small demonstration of flexibility):

.. code-block:: none

	static int const* function* function* a [2] () (int);

There are no nested declarators in Jancy. Nested declarators in C/C++ emerged as a solution (and in my personal opinion, not an elegant one) to the problem of resolving ambiguities in complex pointer-to-function declarations. Like you just saw, Jancy uses a different approach with type modifiers ``function``, ``property``, ``array``:

.. code-block:: none

	int property* function* array* a [2] [3] ();

Here ``a`` is an array of three elements of type **pointer to array of two elements of type pointer to a function taking no arguments and returning a pointer to int property**. Speaking formally, the rules for reading Jancy declaration are as follows. First, you start unrolling declarator's pointer prefixes right-to-left. If type modifiers of a pointer prefix requires a suffix, you unroll the first suffix. After all pointer prefixes are unrolled, you unroll the remaining suffixes.

In reality, however, you are unlikely going to need mind-boggling declarations like the one above. It's always possible to split an overcomplicated declaration into two (or more) using good-old typedefs: just like in C/C++, Jancy declarations with ``typedef`` storage specifier result in creation of a type alias:

.. code-block:: none

	typedef double DoubleBinaryFunc (double, double);
	DoubleBinaryFunc* funcPtr; // use new typedef to declare a variable

	typedef int IntArray [10] [20];
	IntArray foo ();  // use new typedef as a retval type

There are other important differences with C/C++. In Jancy named type declaration is **not** a type specifier. The following code, perfectly valid in C/C++ will produce an error in Jancy:

.. code-block:: none

	struct Point
	{
		int m_x;
		int m_y;
	} point;

In Jancy you cannot declare a named type and immediatly use it to declare a variable or a field. Therefore, to fix previous example, we need to simply split a single declaration into two:

.. code-block:: none

	struct Point
	{
		int m_x;
		int m_y;
	}

	Point point;

Note that declaration of a named type does not need to end with a semicolon (needless to say, it will also compile should you add a semicolon).

Jancy does not require **declaration-before-usage** at global scope. Therefore, there is no need to create so-called **forward declarations** of functions or types, so the following example will compile in Jancy, but not in C/C++:

.. code-block:: none

	void foo ()
	{
		A a;
		B b;
	}

	struct A
	{
		B* m_b;
	}

	struct B
	{
		A* m_a;
	}

It is allowed to omit type specifier; ``void`` type is assumed in this case. This is done to unify rules applied to declaration of normal functions and **special** functions like **constructors**, **destructors**, **setters** etc. In Jancy the following two declarations are equivalent:

.. code-block:: none

	void foo ()
	{
		//...
	}

	foo ()
	{
		//...
	}
