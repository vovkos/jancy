// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {

class Type;
class Runtime;
struct Box;

//.............................................................................

// structure backing up fat data pointer, e.g.:
// int* p;

struct DataPtrValidator
{
	Box* m_validatorBox;
	Box* m_targetBox;
	const void* m_rangeBegin;
	size_t m_rangeLength;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DataPtr
{
	void* m_p;
	DataPtrValidator* m_validator;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SELECT_ANY DataPtr g_nullPtr = { 0 };

//.............................................................................

// every class instance, every gc-allocated block and every static variable
// whose address has been used by a safe pointer needs a box

enum BoxFlag
{
	BoxFlag_WeakMark        = 0x01,
	BoxFlag_ClosureWeakMark = 0x02,
	BoxFlag_DataMark        = 0x04,
	BoxFlag_ClassMark       = 0x08,
	BoxFlag_Zombie          = 0x10,
	BoxFlag_StaticData      = 0x20,
	BoxFlag_DynamicArray    = 0x40,

	BoxFlag_MarkMask        = 0x0f,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Box
{
	Type* m_type;

	uintptr_t m_flags      : 8;

#if (_AXL_PTR_BITNESS == 64)
	uintptr_t m_rootOffset : 56;
#else
	uintptr_t m_rootOffset : 24; // more than enough
#endif
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct StaticDataBox: Box
{
	void* m_p; // static data boxes are detached from the actual data
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DynamicArrayBoxHdr: Box
{
	union
	{
		size_t m_count;
		uint64_t m_padding; // ensure 8-byte alignment
	};
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DataBox: Box
{
	DataPtrValidator m_validator;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DynamicArrayBox: DynamicArrayBoxHdr
{
	DataPtrValidator m_validator;
};

//.............................................................................

// header of class iface

struct IfaceHdr
{
	void* m_vtable;
	Box* m_box;

	IfaceHdr () // empty constructor to prevent C++ from touching iface headers after prime
	{
	}

	// followed by parents, then by iface data fields
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// iface inside a box

template <typename T>
class ClassBox: public Box
{
protected:
	char m_buffer [sizeof (T)];

public:
	T* p ()
	{
		return (T*) m_buffer;
	}

	operator T* ()
	{
		return p ();
	}

	T* operator -> ()
	{
		return p ();
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef 
void
StaticConstructFunc ();

typedef 
void
StaticDestructFunc ();

typedef 
void
DestructFunc (IfaceHdr* iface);

//.............................................................................

// structure backing up fat function pointers, e.g.:
// int function* pfTest (int, int);
// int function weak* pfTest (int, int);

struct FunctionPtr
{
	void* m_p;
	IfaceHdr* m_closure;
};

//.............................................................................

// structure backing up property pointers, e.g.:
// int property* pxTest;
// int property weak* pxTest;

struct PropertyPtr
{
	void** m_vtable;
	IfaceHdr* m_closure;
};

//.............................................................................

enum VariantField
{
	VariantField_Data1,
	VariantField_Data2,
#if (_AXL_PTR_SIZE == 4)
	VariantField_Padding,
#endif
	VariantField_Type,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// structure backing up variants, e.g.:
// variant v;

struct Variant
{
	union 
	{
		int8_t m_int8;
		uint8_t m_uint8;
		int16_t m_int16;
		uint16_t m_uint16;
		int32_t m_int32;
		uint32_t m_uint32;
		int64_t m_int64;
		uint64_t m_uint64;
		intptr_t m_intptr;
		uintptr_t m_uintptr;

		float m_float;
		double m_double;

		void* m_p;
		IfaceHdr* m_classPtr;
		DataPtr m_dataPtr;
		FunctionPtr m_functionPtr;
		PropertyPtr m_propertyPtr;
	};

#if (_AXL_PTR_SIZE == 4)
	char m_padding [4]; // ensure the same layout regardless of pack factor
#endif

	Type* m_type;
};

AXL_SELECT_ANY Variant g_nullVariant = { 0 };

//.............................................................................

// structures backing up multicast, e.g.:
// mutlicast OnFire ();

struct Multicast: IfaceHdr
{
	volatile intptr_t m_lock;
	DataPtr m_ptr; // array of function closure, weak or unsafe pointers
	size_t m_count;
	size_t m_maxCount;

	void* m_handleTable;
};

//.............................................................................

// multicast snapshot returns function pointer with this closure:

struct McSnapshot: IfaceHdr
{
	DataPtr m_ptr; // array of function closure or unsafe pointers
	size_t m_count;
};

//.............................................................................

// structure backing up reactor bind site in reactor class

struct ReactorBindSite
{
	IfaceHdr* m_onChanged;
	intptr_t m_cookie;
};

//.............................................................................

// structure backing up formatting literal

struct FmtLiteral
{
	DataPtr m_ptr;
	size_t m_length;
	size_t m_maxLength;
};

//.............................................................................

struct GcShadowStackFrameMap
{
	size_t m_count;

	// followed by gc root type array;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct GcShadowStackFrame
{
	GcShadowStackFrame* m_prev;
	GcShadowStackFrameMap* m_map;

	// followed by gc root array
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct GcMutatorThread: rtl::ListLink
{
	uint64_t m_threadId;
	size_t m_waitRegionLevel;
	size_t m_noCollectRegionLevel;
	DataPtrValidator* m_dataPtrValidatorPoolBegin;
	DataPtrValidator* m_dataPtrValidatorPoolEnd;
};

//.............................................................................

struct Tls: public rtl::ListLink
{
	Tls* m_prev;
	Runtime* m_runtime;
	size_t m_initializeLevel;
	void* m_stackEpoch;
	GcMutatorThread m_gcMutatorThread;

	// followed by TlsVariableTable
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct TlsVariableTable
{
	GcShadowStackFrame* m_shadowStackTop;

	// followed by user TLS variables 
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Tls*
getCurrentThreadTls ()
{
	return mt::getTlsSlotValue <Tls> ();
}

inline
GcMutatorThread*
getCurrentGcMutatorThread ()
{
	Tls* tls = getCurrentThreadTls ();
	return tls ? &tls->m_gcMutatorThread : NULL;
}

//.............................................................................

struct ExceptionRecoverySnapshot
{
	size_t m_initializeLevel;
	size_t m_waitRegionLevel;
	size_t m_noCollectRegionLevel;
	GcShadowStackFrame* m_gcShadowStackTop;

	bool m_result;
};

//.............................................................................

} // namespace jnc
