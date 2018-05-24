//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#define _JNC_DEF_H

#include "jnc_Pch.h"

//..............................................................................

/**

\defgroup base-def Base Definitions
	\import{jnc_Def.h}

	This section describes base global definitions of Jancy C API.

\addtogroup base-def
@{

*/

//..............................................................................

// when compiling core libraries, we want to use actual implementation classes
// as to avoid unncecessary casts; otherwise, use opaque struct pointers

#ifdef _JNC_CORE

namespace jnc {
namespace ct {

class ModuleItemDecl;
class ModuleItem;
class Attribute;
class AttributeBlock;
class Namespace;
class GlobalNamespace;
class Alias;
class Variable;
class Function;
class Property;
class Typedef;
class Type;
class NamedType;
class BaseTypeSlot;
class DerivableType;
class ArrayType;
class BitFieldType;
class FunctionArg;
class FunctionType;
class PropertyType;
class EnumConst;
class EnumType;
class StructField;
class StructType;
class UnionType;
class ClassType;
class MulticastClassType;
class McSnapshotClassType;
class DataPtrType;
class ClassPtrType;
class FunctionPtrType;
class PropertyPtrType;
class Unit;
class Module;
class GcShadowStackFrameMap;

} // namespace ct

namespace rt {

class Runtime;
class GcHeap;

} // namespace rt
} // namespace jnc

typedef axl::sl::ListLink jnc_ListLink;
typedef axl::sl::Guid jnc_Guid;
typedef axl::err::ErrorHdr jnc_Error;
typedef axl::err::ErrorRouter jnc_ErrorRouter;
typedef jnc::ct::ModuleItemDecl jnc_ModuleItemDecl;
typedef jnc::ct::ModuleItem jnc_ModuleItem;
typedef jnc::ct::Attribute jnc_Attribute;
typedef jnc::ct::AttributeBlock jnc_AttributeBlock;
typedef jnc::ct::Namespace jnc_Namespace;
typedef jnc::ct::GlobalNamespace jnc_GlobalNamespace;
typedef jnc::ct::Alias jnc_Alias;
typedef jnc::ct::Variable jnc_Variable;
typedef jnc::ct::Function jnc_Function;
typedef jnc::ct::Property jnc_Property;
typedef jnc::ct::Typedef jnc_Typedef;
typedef jnc::ct::Type jnc_Type;
typedef jnc::ct::NamedType jnc_NamedType;
typedef jnc::ct::BaseTypeSlot jnc_BaseTypeSlot;
typedef jnc::ct::DerivableType jnc_DerivableType;
typedef jnc::ct::ArrayType jnc_ArrayType;
typedef jnc::ct::BitFieldType jnc_BitFieldType;
typedef jnc::ct::FunctionArg jnc_FunctionArg;
typedef jnc::ct::FunctionType jnc_FunctionType;
typedef jnc::ct::PropertyType jnc_PropertyType;
typedef jnc::ct::EnumConst jnc_EnumConst;
typedef jnc::ct::EnumType jnc_EnumType;
typedef jnc::ct::StructField jnc_StructField;
typedef jnc::ct::StructType jnc_StructType;
typedef jnc::ct::UnionType jnc_UnionType;
typedef jnc::ct::ClassType jnc_ClassType;
typedef jnc::ct::MulticastClassType jnc_MulticastClassType;
typedef jnc::ct::McSnapshotClassType jnc_McSnapshotClassType;
typedef jnc::ct::DataPtrType jnc_DataPtrType;
typedef jnc::ct::ClassPtrType jnc_ClassPtrType;
typedef jnc::ct::FunctionPtrType jnc_FunctionPtrType;
typedef jnc::ct::PropertyPtrType jnc_PropertyPtrType;
typedef jnc::ct::Unit jnc_Unit;
typedef jnc::ct::Module jnc_Module;
typedef jnc::rt::Runtime jnc_Runtime;
typedef jnc::rt::GcHeap jnc_GcHeap;
typedef jnc::ct::GcShadowStackFrameMap jnc_GcShadowStackFrameMap;

#	define JNC_GUID_INITIALIZER AXL_SL_GUID_INITIALIZER
#	define JNC_DEFINE_GUID AXL_SL_DEFINE_GUID
#	define jnc_g_nullGuid axl::sl::g_nullGuid

namespace jnc {

axl::sl::String*
getTlsStringBuffer ();

} // namespace jnc

#else // _JNC_CORE

typedef struct jnc_ModuleItemDecl jnc_ModuleItemDecl;
typedef struct jnc_ModuleItem jnc_ModuleItem;
typedef struct jnc_Attribute jnc_Attribute;
typedef struct jnc_AttributeBlock jnc_AttributeBlock;
typedef struct jnc_Namespace jnc_Namespace;
typedef struct jnc_GlobalNamespace jnc_GlobalNamespace;
typedef struct jnc_Alias jnc_Alias;
typedef struct jnc_Variable jnc_Variable;
typedef struct jnc_Function jnc_Function;
typedef struct jnc_Property jnc_Property;
typedef struct jnc_Typedef jnc_Typedef;
typedef struct jnc_Type jnc_Type;
typedef struct jnc_NamedType jnc_NamedType;
typedef struct jnc_BaseTypeSlot jnc_BaseTypeSlot;
typedef struct jnc_DerivableType jnc_DerivableType;
typedef struct jnc_ArrayType jnc_ArrayType;
typedef struct jnc_BitFieldType jnc_BitFieldType;
typedef struct jnc_FunctionArg jnc_FunctionArg;
typedef struct jnc_FunctionType jnc_FunctionType;
typedef struct jnc_PropertyType jnc_PropertyType;
typedef struct jnc_EnumConst jnc_EnumConst;
typedef struct jnc_EnumType jnc_EnumType;
typedef struct jnc_StructField jnc_StructField;
typedef struct jnc_StructType jnc_StructType;
typedef struct jnc_UnionType jnc_UnionType;
typedef struct jnc_ClassType jnc_ClassType;
typedef struct jnc_MulticastClassType jnc_MulticastClassType;
typedef struct jnc_McSnapshotClassType jnc_McSnapshotClassType;
typedef struct jnc_DataPtrType jnc_DataPtrType;
typedef struct jnc_ClassPtrType jnc_ClassPtrType;
typedef struct jnc_FunctionPtrType jnc_FunctionPtrType;
typedef struct jnc_PropertyPtrType jnc_PropertyPtrType;
typedef struct jnc_Unit jnc_Unit;
typedef struct jnc_Module jnc_Module;
typedef struct jnc_Runtime jnc_Runtime;
typedef struct jnc_GcHeap jnc_GcHeap;
typedef struct jnc_GcShadowStackFrameMap jnc_GcShadowStackFrameMap;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#	ifdef _AXL_SL_LISTBASE_H

typedef axl::sl::ListLink jnc_ListLink;

#	else // _AXL_SL_LISTBASE_H

typedef struct jnc_ListLink jnc_ListLink;

/**
	\brief This struct holds information about a single entry of a doubly linked list.
	\verbatim

	Whenever something needs to be put on a Jancy list, it must be represented as a struct with ``jnc_ListLink`` memeber field in it. If a list contains entries of non-aggregate type (such as ``int``, or ``void*``), then we have to *box* entry in a struct as such:

	.. code-block:: c

		struct IntEntry
		{
			jnc_ListLink m_link; // <-- list link
			int m_value;
		};

	However, often times what you want to put on a list -- is already an aggregate. In this case just embed a ``jnc_ListLink`` member field into the struct as such:

	.. code-block:: c

		struct MyItem
		{
			jnc_ListLink m_link; // <-- list link

			const char* m_name;
			const char* m_description;
			int m_state;

			// ... more fields
		};

	\endverbatim
*/

struct jnc_ListLink
{
	jnc_ListLink* m_next; ///< Holds a pointer to the *next* list entry on the list or ``NULL`` if this is the *last* entry.
	jnc_ListLink* m_prev; ///< Holds a pointer to the *previous* list entry on the list or ``NULL`` if this is the *first* entry.
};

#	endif // _AXL_SL_LISTBASE_H

#	ifdef _AXL_SL_GUID_H

typedef axl::sl::Guid jnc_Guid;

#		define JNC_GUID_INITIALIZER AXL_SL_GUID_INITIALIZER
#		define JNC_DEFINE_GUID AXL_SL_DEFINE_GUID
#	else // _AXL_SL_GUID_H

typedef struct jnc_Guid jnc_Guid;

/**
	\brief This struct describes a globally unique identifier (GUID).
	\verbatim

	GUIDs are 128-bit values often used as permanent labels which uniquely identify some sort of information. Creation of GUIDs does not require any significant central coordination.

	In practice any two generated GUIDs could be assumed to be distinct -- even though neither of GUID generation algorithms could strictly guarantee uniqueness of generated identifiers. The probability of collision is too low and can be neglected in most practical applications.

	In Jancy API GUIDs are used to identify extension libraries -- each extension library has its own unique ID.

	This struct directly maps to ``struct UUID`` as defined in WinAPI.

	For more details about globally unique identifiers refer to: https://en.wikipedia.org/wiki/Globally_unique_identifier

	\endverbatim
*/

struct jnc_Guid
{
	union
	{
		struct
		{
			/**
				\unnamed{union/struct:4}
				\subgroup

				The *classic* representation of a ``GUID`` mirroring the definition in WinAPI.
			*/

			uint32_t m_data1;
			uint16_t m_data2;
			uint16_t m_data3;
			uint8_t m_data4 [8];
		};

		struct
		{
			/**
				\unnamed{struct:4}
				\subgroup

				An *alternative* representation of a ``GUID`` as four 32-bit integers.
			*/

			uint32_t m_long1;
			uint32_t m_long2;
			uint32_t m_long3;
			uint32_t m_long4;
		};
	};
};

/// \cond _EXCLUDED

#		ifdef __cplusplus
#			define JNC_GUID_SPECIFIER extern JNC_SELECT_ANY const
#		else
#			define JNC_GUID_SPECIFIER JNC_SELECT_ANY const
#		endif

#		define JNC_GUID_INITIALIZER(l, s1, s2, b1, b2, b3, b4, b5, b6, b7, b8) \
			{ { { l, s1, s2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } } } }

/// \endcond

/**
	\verbatim

	A cross-platform equivalent of the Win32 ``DEFINE_GUID`` macro.

	This macro is used to define a constant holding the **GUID** (Globally Unique Identifier).

	To generate a new GUID use any online GUID-creation tool and select the ``DEFINE_GUID`` format; then replace ``DEFINE_GUID`` with ``JNC_DEFINE_GUID``.

	.. rubric:: Sample:

	.. code-block:: cpp

		// {384498AC-90AF-4634-B083-2A9B02D62680}

		JNC_DEFINE_GUID (
			g_testLibGuid,
			0x384498ac, 0x90af, 0x4634, 0xb0, 0x83, 0x2a, 0x9b, 0x2, 0xd6, 0x26, 0x80
			);

	\endverbatim
*/

#		define JNC_DEFINE_GUID(n, l, s1, s2, b1, b2, b3, b4, b5, b6, b7, b8) \
			JNC_GUID_SPECIFIER jnc_Guid n = \
			JNC_GUID_INITIALIZER (l, s1, s2, b1, b2,  b3,  b4,  b5,  b6,  b7,  b8)
#	endif // _AXL_SL_GUID_H

#	ifdef _AXL_ERR_ERROR_H

typedef axl::err::ErrorHdr jnc_Error;

#	else

typedef struct jnc_Error jnc_Error;

struct jnc_Error
{
	uint32_t m_size;
	jnc_Guid m_guid;
	uint32_t m_code;
};

#	endif // _AXL_ERR_ERROR_H

#	ifdef _AXL_ERR_ERRORMGR_H

typedef axl::err::ErrorRouter jnc_ErrorRouter;

#	else

typedef struct jnc_ErrorRouter jnc_ErrorRouter; // opaque

#	endif // _AXL_ERR_ERRORMGR_H

#endif // _JNC_CORE

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef struct jnc_ExtensionLib jnc_ExtensionLib;
typedef struct jnc_GcStats jnc_GcStats;
typedef struct jnc_GcSizeTriggers jnc_GcSizeTriggers;

//..............................................................................

JNC_EXTERN_C
void
jnc_initialize (const char* tag);

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_ListLink ListLink;
typedef jnc_Guid Guid;
typedef jnc_Error Error;
typedef jnc_ModuleItemDecl ModuleItemDecl;
typedef jnc_ModuleItem ModuleItem;
typedef jnc_Attribute Attribute;
typedef jnc_AttributeBlock AttributeBlock;
typedef jnc_Namespace Namespace;
typedef jnc_GlobalNamespace GlobalNamespace;
typedef jnc_Alias Alias;
typedef jnc_Variable Variable;
typedef jnc_Function Function;
typedef jnc_Property Property;
typedef jnc_Typedef Typedef;
typedef jnc_Type Type;
typedef jnc_NamedType NamedType;
typedef jnc_BaseTypeSlot BaseTypeSlot;
typedef jnc_DerivableType DerivableType;
typedef jnc_ArrayType ArrayType;
typedef jnc_BitFieldType BitFieldType;
typedef jnc_FunctionArg FunctionArg;
typedef jnc_FunctionType FunctionType;
typedef jnc_PropertyType PropertyType;
typedef jnc_EnumConst EnumConst;
typedef jnc_EnumType EnumType;
typedef jnc_StructField StructField;
typedef jnc_StructType StructType;
typedef jnc_UnionType UnionType;
typedef jnc_ClassType ClassType;
typedef jnc_MulticastClassType MulticastClassType;
typedef jnc_McSnapshotClassType McSnapshotClassType;
typedef jnc_DataPtrType DataPtrType;
typedef jnc_ClassPtrType ClassPtrType;
typedef jnc_FunctionPtrType FunctionPtrType;
typedef jnc_PropertyPtrType PropertyPtrType;
typedef jnc_Unit Unit;
typedef jnc_Module Module;
typedef jnc_Runtime Runtime;
typedef jnc_GcHeap GcHeap;
typedef jnc_GcShadowStackFrameMap GcShadowStackFrameMap;
typedef jnc_ExtensionLib ExtensionLib;
typedef jnc_GcStats GcStats;
typedef jnc_GcSizeTriggers GcSizeTriggers;

//..............................................................................

inline
void
initialize (const char* tag = NULL)
{
	jnc_initialize (tag);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
