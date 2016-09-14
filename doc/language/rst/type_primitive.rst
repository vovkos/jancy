Primitive Types
===============

System of primitive types in Jancy is a lot like the one in C/C++ with one exception: there is native support for integers with reversed byte-order (aka **bigendians**).

Bigendians are widely used throughout most network protocol stacks and everyone who was ever involved in network programming knows two things: number one, it kind of clutters the programs to constantly reverse byte order manually with all these **ntohs/htons/ntohl/htonl** calls. Even more importantly, number two -- it's **sooo** easy to forget to reverse byte order, thus getting a program which compiles just fine but has a logical bug in it! Since bigendians are so abundant in networking, and Jancy is intended for low-level IO programmers, bigendians got native support in Jancy.

Below is a list of all primitive types in Jancy.

* ``void``
* ``bool``
* ``char`` (``int8_t``)
* ``unsigned char`` (``uint8_t``, ``uchar_t``, ``byte_t``)
* ``short`` (``int16_t``)
* ``unsigned short`` (``uint16_t``, ``ushort_t``, ``word_t``)
* ``int`` (``int32_t``)
* ``unsigned int`` (``uint32_t``, ``uint_t``, ``dword_t``)
* ``long`` (``int64_t``)
* ``unsigned long`` (``uint64_t``, ``ulong_t``, ``qword_t``)
* ``bigendian short``
* ``bigendian unsigned short``
* ``bigendian int``
* ``bigendian unsigned int``
* ``bigendian long``
* ``bigendian unsigned long``
* ``intptr`` (aliases to ``int32_t`` or ``int64_t`` depending on platform)
* ``unsigned intptr`` (aliases to ``uint32_t`` or ``uint64_t`` depending on platform)
* ``float``
* ``double``
