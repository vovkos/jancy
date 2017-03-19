Jancy IDE Modifications
=======================

Lexer
-----

Tokens have changed. Refer to ``jnc_ct_Lexer.rl`` for reference. Some notes:

* Integer type keywords have been brought to consistence with C/C++:

	- ``char`` (8 bit)
	- ``short`` (16 bit)
	- ``int`` (32 bit)
	- ``long`` (64 bit)
	- ``intptr`` (32/64 bit)

	Other integer keywords (``int8``, ``int16``, etc) are removed and everything else is typedef-ed (``int8_t``, ``uint16_t``, etc).

* ``anydata`` type modifier. This change in Jancy language was done long time ago, but since it's not used in IO Ninja scripts, I never have really noticed the lack of support for it in the IDE. ``anydata`` type specifier was added to create versatile (both POD and non-POD) data pointers (``void*`` is an abstact POD-pointer and can't be used to store pointers to non-POD-data. With ``anydata`` pointers you can store both -- at the cost of required ``dynamic`` casts). IDE-wise -- just add ``anydata`` type specifier and make it behave the same as ``void``.

* ``automaton`` removed, ``reswitch`` added (more on that later).

* ``%% ...`` regex literals are gone, raw literals ``[rR]"..."`` are added. Raw literals are similar to Python raw literals (without escape decoding) are primarily dedicated for declaring regular expressions in ``reswitch`` statements. They may also come handly for declaring Windows paths -- no need to escape all ``\``-s or replace  them with ``/``-s.

* Binary literals with decimal radix ``0x[nNdD]"..."`` are in Jancy for quite a while already, but apparently are not supported in the IDE. It's pretty convenient to declare IP address constants with decimal binary literals: ``0n"127 0 0 1"``, ``0n"192 168 1 32", etc. Binary literals with octal and binary radix ``0x[oO]"..."``, ``0x[bB]"..."`` added just for consistency (don't see the point in octal radix tbh, but octals are part of the C/C++ standard)

Aliases
-------

Alias syntax and semantics has changed.

Before aliases were a form of expression macros and had to be evaluated in the place of application. The way it was used for sharing events between bindable properties was kind of a dirty hack (implementation, that is. The actual feature of event sharing kicks ass!). I decided to simplify everything greately by reducing the scope of aliases to qualified name. Now alias is just... well, an alias for some named entity! It is evaluated at the calc-layout stage and can be used for ``bindable`` event and ``autoget`` field sharing -- just as before (but cleaner implementation wise). Type for an alias is completely optional (it's not even checked now, but the check should be implemented... someday).

Checked the IDE and it doesn't work properly::

	alias Alias = io.File;
	Alias a; // underlined as an error

	alias p = printf;
	p ( // no argument tip here

Function Redirection
--------------------

Now it's possible to redirect functions (to share implementation) using initializer syntax. This feature is somewhat reminiscent of aliases (both redirect), but not quite the same semantically. Alias is just a name which maps (redirects) to another name. Consider overloaded function. What if we want to redirect a particular overload? Or what if we want to redirect an unnamed function, such as a destructor or a getter? This obviously can't be done with aliases. But this now can be done with function redirection, and the syntax can't be any better! Initializers for functions -- are what these functions are redirected to.

Again, checked the IDE and it doesn't work properly::

	class Class
	{
		destruct () = close; // underlined as an error

		close ();
	}

Declaration Grammar
-------------------

Reworked declaration grammar, and now it's possible to... drum roll... omit semicolon after curly initializers! How cool is that? Not sure about you, but I constantly forget to terminate array initializers in C/C++. If you think about it -- it's so inconsistent! Function bodies don't have to be terminated, compound or ``switch`` statements don't have to be terminated. But named types and curly initializers do! Well, I took care of named type termination long time ago. The last piece of the puzzle was alleviating this requirement for curly initializers, and I tell you -- it wasn't that easy!

On details how to do that -- check ``jnc_ct_Decl.llk`` and ``jnc_ct_Declarator.llk``. Not sure if it will map directly to JavaCC though -- it involves ``anytoken`` lists for collecting curly initializers (can't use expression_pass1 for initalizers anymore). When you finish implementing that, make sure declarator lists still work (i.e. it's possible to write ``, <next declartor>`` after ``}``).

Regex Switches
--------------

Big change in lexer generator. I kept thinking on how to improve automaton functions without compromising the functionality they provide. I feel the concept of automaton functions -- a built-in Ragel/Lex/Flex -- is great, but it's a bit hard to wrap your head around it -- how to invoke automatons, return values from automatons, relations between automatons and recognizers etc. The solution I finally settled on kicks major ass::

		jnc.RegexState state; // may be re-used in a loop, upon receiving the next chunk over IO stream, etc
		reswitch (state, string)
		{
		case r"foo\s+\d+": // raw literal to prevent escape expansion as regex-es use different rules
			// OK to fall-through to next case, just like in a regular switch

		case "bar": // if no regex escapes are used -- regular literal will do just fine
			// ...
			break;

		default:
			// mismatch
		}

It's OK to pass the string length as the third parameter, it's OK to re-enter the switch in a loop to process the whole string (adjusting the string pointer), and it's OK to continue process the stream chunk-by-chunk like before with automatons (``state.m_isIncremental = true``).

I believe ``reswitch`` a huge step forward in terms of *being easy to understand*. And just like automatons it remains a unique feature -- no other language provides regex-based switches. To be 100% correct I have to add that it's possible to do something similar to regex switch in recent versions of ECMA script, but it can't work chunk-by-chunk and more importantly, it will essentially result in a sequence of regex matches, i.e. complexity will be O(input-length * number-of-cases), while in Jancy it's a single DFA i.e. O(input-length).

For syntactic details check ``jnc_ct_Stmt.llk``, for sample code check ``70_RegExSwitch.jnc`` in Jancy sample folder or ``io_UsbDb.jnc`` in IO Ninja common script folder.

Forced Imports in Extension Libraries
-------------------------------------

I implemented the feaure we discussed the other day. Now the information about whether a particular import should be forced or not is contained withing the file name. If it starts with a dot ``.`` -- it's a dedicated file with the declarations for the IDE and should be processed automatically (forced). These ``.``-prefixed files are ignored by the Jancy import manager (extension libs can force-import directly via Jancy API).

Obviously, there is no need to have more than one such file per extension library -- this single file will contain all the necessary declarations and imports. Now all files like that are called ``.forced.jnc``, so you can do a regular string compare, but I think checking for the dot-prefix is a better approach than comparing the whole file name.

Also, if there is a single ``.jnc`` source file in ``.jncx`` dynamic extension library, process it unconditionally. Yes, we can add ``.forced.jnc`` and import the actual source file from there, but why? I can't think of any reason why the single file should not be auto-imported...

And since now we have a tool to specify forced-imported sources, we can finally move all the stdlib sources to a dedicated folder and make it configurable, so there is no need to rebuild the plugin (or modify it manually with 7zip)!

Navigation For Library Items
----------------------------

Every time I use IO Ninja IDE I find myself ctrl-clicking on some out-of-project class or method (e.g. ``std.Error`` or ``io.Socket.connect``) only to find out it's not possible. We need to implement navigation for library sources.

It should work like this. Whenever you get to parse a source file from the dynamic extension library (not sure how IDE is designed -- do you parse it upon discovering an appropriate ``.jnc`` import or parse all sources belonging to a particular ``.jncx`` right away?) you should save it in an appropriately named file in some unique temp folder. You likely will need to maintain a map ``.jncx`` => folder; this map and temp folder structure can be persistent or generated on each IDE run -- you will know better. Anyway, whenever user ctrl-clicks on an item defined in ``.jncx``, you should open this file from a temp folder in read-only mode. Not sure if possible, but it would also be nice to show where this file is coming from (via tooltip on a tab?)

Error Recovery
--------------

Noticed that if some error is detected at global scope, everything below is not parsed. I think recovery in parser worked before?
