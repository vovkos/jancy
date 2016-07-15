#pragma once

//.............................................................................

#ifdef _MSC_VER
#	define JNC_SELECT_ANY __declspec (selectany)
#elif (defined __GNUC__)
#	define JNC_SELECT_ANY  __attribute__ ((weak))
#else
#	error unsupported compiler
#endif

#ifdef __cplusplus 
#	define JNC_EXTERN_C extern "C"
#else
#	define JNC_EXTERN_C
#endif

// inheriting which works for both C and C++

#ifdef __cplusplus 
#	define JNC_BEGIN_INHERITED_STRUCT(Struct, BaseStruct) \
	struct Struct: BaseStruct {
#else
#	define JNC_BEGIN_INHERITED_STRUCT (Struct, BaseStruct) \
	struct Struct { BaseStruct;
#endif

#define JNC_END_INHERITED_STRUCT() \
	};

//.............................................................................

// when compiling core libraries, we want to use actual implementation classes
// as to avoid unncecessary casts; otherwise, use opaque struct pointers

#ifdef _JNC_CORE

namespace jnc {
namespace ct {

class ModuleItem;
class Namespace;
class Type;
class DerivableType;
class ClassType;
class Function;
class Property;
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
typedef jnc::ct::ModuleItem jnc_ModuleItem;
typedef jnc::ct::Namespace jnc_Namespace;
typedef jnc::ct::Type jnc_Type;
typedef jnc::ct::DerivableType jnc_DerivableType;
typedef jnc::ct::ClassType jnc_ClassType;
typedef jnc::ct::Function jnc_Function;
typedef jnc::ct::Property jnc_Property;
typedef jnc::ct::Module jnc_Module;
typedef jnc::rt::Runtime jnc_Runtime;
typedef jnc::rt::GcHeap jnc_GcHeap;
typedef jnc::ct::GcShadowStackFrameMap jnc_GcShadowStackFrameMap;

#	define JNC_GUID_INITIALIZER AXL_SL_GUID_INITIALIZER
#	define JNC_DEFINE_GUID AXL_SL_DEFINE_GUID
#	define jnc_g_nullGuid axl::sl::g_nullGuid

#else // _JNC_CORE

typedef struct jnc_Error jnc_Error;
typedef struct jnc_ModuleItem jnc_ModuleItem;
typedef struct jnc_Namespace jnc_Namespace;
typedef struct jnc_Type jnc_Type;
typedef struct jnc_DerivableType jnc_DerivableType;
typedef struct jnc_ClassType jnc_ClassType;
typedef struct jnc_Function jnc_Function;
typedef struct jnc_Property jnc_Property;
typedef struct jnc_Module jnc_Module;
typedef struct jnc_Runtime jnc_Runtime;
typedef struct jnc_GcHeap jnc_GcHeap;
typedef struct jnc_GcShadowStackFrameMap jnc_GcShadowStackFrameMap;

#	ifdef _AXL_SL_LISTBASE_H

typedef axl::sl::ListLink jnc_ListLink;

#	else // _AXL_SL_LISTBASE_H

typedef struct jnc_ListLink jnc_ListLink;

struct jnc_ListLink
{
	jnc_ListLink* m_next;
	jnc_ListLink* m_prev;
};

#	endif // _AXL_SL_LISTBASE_H

#	ifdef _AXL_SL_GUID_H

typedef axl::sl::Guid jnc_Guid;

#		define JNC_GUID_INITIALIZER AXL_SL_GUID_INITIALIZER
#		define JNC_DEFINE_GUID AXL_SL_DEFINE_GUID

#	else // _AXL_SL_GUID_H

typedef struct jnc_Guid jnc_Guid;

struct jnc_Guid
{
	union
	{
		struct
		{
			uint32_t m_data1;
			uint16_t m_data2;
			uint16_t m_data3;
			uint8_t m_data4 [8];
		};

		struct
		{
			uint32_t m_long1;
			uint32_t m_long2;
			uint32_t m_long3;
			uint32_t m_long4;
		};
	};
};

#		define JNC_GUID_INITIALIZER(l, s1, s2, b1, b2, b3, b4, b5, b6, b7, b8) \
			{ { { l, s1, s2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } } } }

#		define JNC_DEFINE_GUID(n, l, s1, s2, b1, b2, b3, b4, b5, b6, b7, b8) \
			extern JNC_SELECT_ANY const jnc_Guid n = \
			JNC_GUID_INITIALIZER (l, s1, s2, b1, b2,  b3,  b4,  b5,  b6,  b7,  b8)
#	endif // _AXL_SL_GUID_H

#endif // _JNC_CORE

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_ListLink ListLink;
typedef jnc_Guid Guid;
typedef jnc_Error Error;
typedef jnc_ModuleItem ModuleItem;
typedef jnc_Namespace Namespace;
typedef jnc_Type Type;
typedef jnc_DerivableType DerivableType;
typedef jnc_ClassType ClassType;
typedef jnc_Function Function;
typedef jnc_Property Property;
typedef jnc_Module Module;
typedef jnc_Runtime Runtime;
typedef jnc_GcHeap GcHeap;
typedef jnc_GcShadowStackFrameMap GcShadowStackFrameMap;

//.............................................................................

} // namespace jnc

#endif // __cplusplus
