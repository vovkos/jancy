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

Dynamic Structs
===============

Jancy is not the first language to introduce *dynamic structs* per se -- many languages support introspection/reflection facilities, which allow modifying the type system at runtime. Making them, you know, dynamic.

However, dynamic structs in Jancy allow something that's simply not possible with any other language -- and that is, self-description. The lengths of some fields may be non-constanct and expressed as a function of other fields of the same struct (e.g. "length"/"count" fields), by null-termination, etc. This approach is used in many file formats and network protocols (think of TCP/IP variable-length fields).

.. rubric:: Example:

.. code-block:: jnc

	dynamic struct FileHdr
	{
	    uint32_t m_signature;
	    uint32_t m_sectionCount;
	    Section m_sectionTable [m_sectionCount];
	    uint32_t m_version;
	    char m_authorName [strlen (m_authorName) + 1];
	    char m_authorEmail [strlen (m_authorEmail) + 1];
	}

After defining a dynamic struct, just overlap a pointer to it with a buffer and access the fields normally, just the way you would with a regular C struct:

.. code-block:: jnc

	FileHdr	const* hdr = (FileHdr const*) buffer;

	printf (
	    "signature:   %.4s (0x%08x)\n"
	    "sections:    %d\n"
	    "version:     %d.%d.%d (0x%06x)\n"
	    "author:      %s\n"
	    "email:       %s\n",
	    &hdr.m_signature,
	    hdr.m_signature,
	    hdr.m_sectionCount,
	    (hdr.m_version & 0xff0000) >> 16,
	    (hdr.m_version & 0x00ff00) >> 8,
	    (hdr.m_version & 0x0000ff),
	    hdr.m_version,
	    hdr.m_authorName,
	    hdr.m_authorEmail
	    );

All calculated offsets are cached and then re-used during the next access, so you don't have to worry much about the performance aspect of dynamic structs.

You can even fill the buffer using a non-const dynamic struct pointer. Just make sure you assign the fields in correct order (usually, top-to-bottom), so that dynamic offsets could be calculated correctly.

To get the actual size of a dynamic struct, you can apply a ``dynamic sizeof`` operator:

.. code-block:: jnc

	size_t size = dynamic sizeof (*p);

.. rubric:: Restrictions:

Of course, given the fact that the size of such struct is dynamically calculated, there are natural limitations applied:

* Can't allocate a dynamic struct, can only have a pointer to it;
* Pointer subtraction is illegal;
* Pointer increment is only allowed for ``+1`` delta, e.g.:

	.. code-block:: jnc

	    p++;

Another thing to consider when using dynamic structs is this. Dynamic offsets are cached for efficiency, as has been said above. However, this cache is a double-edged sword when you need to re-use the same buffer for different data. The cache, of course, knows nothing about that, so it will keep reporting the now-erroneous offsets.

To combat that, be sure to reset the cache with ``jnc.resetDynamicLayout`` before re-using the same buffer with new data:

.. code-block:: jnc

	memcpy (buffer, newData, newDataSize);
	jnc.resetDynamicLayout (buffer);

	FileHdr	const* hdr = (FileHdr const*) buffer;

	// offsets will be re-calculated on access
