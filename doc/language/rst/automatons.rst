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

Automaton Functions
===================

Jancy features automaton functions to provide a built-in support for creating protocol analyzers, text scanners, lexers and other recognizers.

If you ever used tools like **Lex**, **Flex**, **Ragel** etc then you are already familiar with the idea. If not, then it is pretty simple, actually. First, you define a list of recognized lexemes in the form of regular expressions. Then you specify which actions to execute when these lexemes are found in the input stream. Jancy compiler will then automatically build a DFA to recognize your language.

.. code-block:: jnc

	jnc.AutomatonResult automaton fooBar (jnc.Recognizer* recognizer)
	{
	    %% "foo"
	        // lexeme found: foo;

	    %% "bar"
	        // lexeme found: bar;

	    %% [0-9]+
	        // lexeme found: decimal-number

	        char const* numberString = recognizer.m_lexeme;

	    %% [ \r\n\t]+
	        // ignore whitespace
	}

Automaton functions cannot be directly called -- you need a recognizer object of type jnc.Recognizer to store the state of DFA and manage accumulation and matching of the input stream.

.. code-block:: jnc

	jnc.Recognizer recognizer (fooBar);

Class jnc.Recognizer features a method **recognize** to do recognition in one go:

.. code-block:: jnc

	bool result = try recognizer.recognize ("foo bar 100 baz");
	if (!result)
	{
	    // handle recognition error
	}

Even more importantly, it's also OK to perform recognition incrementally -- chunk by chunk. This is cructial when analyzing protocols operating over stream transports like TCP or Serial, where it is not guaranteed that a message will be delivered as a whole and not as multiple segments.

.. code-block:: jnc

	try
	{
	    recognizer.write (" ba");
	    recognizer.write ("r f");
	    recognizer.write ("oo ");
	    recognizer.write ("100");
	    recognizer.write ("000");

	    // notify recognizer about eof (this can trigger actions or errors)

	    recognizer.eof ();

	catch:
	    // handle recognition error
	}

Like Ragel, Jancy-generated recognizer support mixed-language documents. Developer can switch languages at will, by adjusting the value of field **m_automatonfunc** at appropriate locations.

In the sample below the first automaton recognizes lexeme ``foo`` and switches to the second automaton upon discovering an opening apostrophe:

.. code-block:: jnc

	jnc.AutomatonResult automaton foo (jnc.Recognizer* recognizer)
	{
	    %% "foo"
	        // lexeme found: foo

	    %% '\''
	        recognizer.m_automatonFunc = bar; // switch language

	    %% [ \r\n\t]+
	        // ignore whitespace
	}

The second automaton recognizes lexeme ``bar`` and switches back to the first automaton when a closing apostrophe is found if and only if it's not escape-protected by a backslash prefix.

.. code-block:: jnc

	jnc.AutomatonResult automaton bar (jnc.Recognizer* recognizer)
	{
	    %% "bar"
	        // lexeme found: bar

	    %% "\\'"
	        // ignore escape-protected apostrophe

	    %% '\''
	        recognizer.m_automatonFunc = foo; // switch language back

	    %% [ \r\n\t]+
	        // ignore whitespace
	}

Of course it's possible to maintain a call stack of previous automaton function pointers and thus implement a recognizer for nested language documents of arbitrary complexity.
