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

Regex Switches
==============

Jancy features efficient regular expression switches for creating protocol analyzers, text scanners, lexers and other recognizers.

If you ever used tools like **Lex**, **Flex**, **Ragel** etc then you are already familiar with the idea. First, you define a list of recognized lexemes in the form of regular expressions. Then you specify which actions to execute when these lexemes are found in the input stream.

Jancy compiler will then automatically build a DFA to find these lexemes in an input stream:

.. code-block:: jnc

	jnc.RegexState state;
	switch (state, string_t(p, length)) {
	case "foo":
		// ...
		break;

	case r"bar(\d+)":
		print ($"bar id: $(state.m_captureArray[0].m_text)\n");
		break;

	case r"\s+":
		// ignore whitespace
		break;

	// ...
	}

Note that unless a regular expression contains no escape sequences or standard character classes (such as ``\d``, ``\h``, ``\s``, etc) it's recommended to use *raw literals* (e.g. ``r"\s*(\d+)\s*"``). This way you can write regular expressions more naturally and avoid the need to escape backslashes (a.k.a *double-backslash madness*).

Incremental Recognition
-----------------------

To employ incremental recognition, you would want to create some *match* function to be called each time the next chunk of data becomes available:

.. code-block:: jnc

	bool errorcode match (
		jnc.RegexState* state,
		const char* p,
		size_t length
	) {
		const char* end = p + length;

		// post-condition loop allows passing 'null' as eof
		do {
			switch (state, string_t(p, end - p)) {
			case "foo":
				// ...
				break;

			case r"bar\(d+)":
				print ($"bar id: $(state.m_captureArray[0].m_text)\n");
				break;

			case r"\s+":
				// ignore whitespace
				break;

			// ...

			default:
				// we can get here for two reasons only:
				//   1) mismatch
				//   2) incremental recognition

				if (!state.m_consumedLength)
					return false;

				assert (state.m_isIncremental && state.m_consumedLength == end - p);
			}

			p += state.m_consumedLength; // advance to the next lexeme
		} while (p < end)

		return true;
	}

Recognizer must be aware of the fact it is being fed the date chunk-by-chunk (and not as the whole) -- certain actions should be performed at the very end, upon the discovery ``EOF``. You make sure recognizer is in *incremental* mode by passing ``true`` into the ``jnc.RegexState`` constructor or by setting ``m_isIncremental`` field to ``true``:

.. code-block:: jnc

	jnc.RegexState state (true);  // turn on incremental matching

	// alternatively, assign:
	// state.m_isIncremental = true;

Now, whenever the next portion of data becomes available, simply call:

.. code-block:: jnc

	size_t length = getNextPortionOfData (buffer);

	match (state, buffer, length);

Pass zero-sized buffer to trigger ``EOF`` processing:

.. code-block:: jnc

	match (state, null, 0);
