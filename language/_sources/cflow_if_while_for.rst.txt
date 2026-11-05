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

if, while, for
==============

These control-flow constructs in Jancy have the same syntax and semantics as in C/C++/C#/Java, so everything should be clear without saying much.

if
--

This statement provides a basic conditional branching. If ``condition`` specified in parenthes of ``if`` statement evaluates to ``true``, then the following statement will be executed. It's also possible to have an ``else`` branch which will be executed if ``condition`` evaluates to ``false``.

.. code-block:: jnc

	if (!result)
		printf($"error: $!\n");

It's also possible to have an ``else`` branch which will be executed if condition evaluates to ``false``.

.. code-block:: jnc

	if (i == 0)
		printf("is 0\n");
	else if (i < 0) {
		printf("is negative\n");
		// ...
	} else {
		printf("is positive\n");
		// ...
	}

while
-----

This loop statement has two forms: *pre-conditional* and *post-conditional*.

In the *pre-conditional* form, ``while`` statement evaluates ``condition`` first. If it evaluates to ``true``, then the following statement (``body-statement``) will be executed, then control will be transferred back for re-evaluation of ``condition``.

.. code-block:: jnc

	while (!m_queue.isEmpty())
		processQueue();

In the *post-conditional* form, ``do-while`` statement executes ``body-statement`` first, then evaluates ``condition``. If it evaluates to ``true`` control is transferred back to ``body-statement``.

.. code-block:: jnc

	bool result;
	do {
		result = processRequest(); // the final request returns false
	} while (result);

for
---

Executes the first statement (``init-statement``) inside the parentheses, then evaluates ``condition``. If it evaluates to ``true``, then the statement following the parenthes (``body-statement``) will be executed. After that control will be transferred to the last statement inside the parentheses  and then back for re-evaluation of ``condition``.

.. code-block:: jnc

	for (int i = 0; i < countof (nameTable); i++) {
		char const* name  = nameTable[i];
		// process name ...
	}

The above ``for`` loop is *completely* equivalent to the following ``while`` loop:

.. code-block:: jnc

	int i = 0;
	while (i < countof (nameTable)) {
		char const* name  = nameTable[i];
		// process name ...
		i++;
	}
