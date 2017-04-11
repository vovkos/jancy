Jancy Grammar Reference
=======================

The present page provides the **full Jancy grammar** for reference purposes.

This listing is **auto-generated** from ``.llk`` files by `Graco <http://github.com/vovkos/graco>`_ parser generator, and is basically the **very same** grammar being used by the Jancy compiler. All the actions, rule attributes and rule arguments removed for readability.

Non-terminals are named using lower-case C-style naming convention (e.g. ``global_declaration``); character terminals are denoted as C ``char`` literals (e.g. ``';'``); non-char terminals have names starting with ``TokenKind_`` prefix (e.g. ``TokenKind_Identifier``).

Grammar Listing
---------------

.. literalinclude:: jancy.llk
	:language: none
