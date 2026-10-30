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

#include "pch.h"
#include "jnc_DynamicLayout.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_Module.h"
#	include "jnc_rt_Runtime.h"
#	include "jnc_rtl_DynamicLayout.h"
#endif

#include "jnc_Runtime.h"

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DynamicLayout*
jnc_createDynamicLayout(
	jnc_Runtime* runtime,
	uint_t mode,
	jnc_DynamicLayoutSink* sink,
	jnc_DataPtr ptr,
	size_t size
) {
	jnc::rtl::DynamicLayout* layout = jnc::createClass<jnc::rtl::DynamicLayout>(runtime);
	layout->reset(mode, (jnc::rtl::DynamicLayoutSink*)sink, ptr, size);
	return (jnc_DynamicLayout*)layout;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicLayout_getSize(jnc_DynamicLayout* layout) {
	return ((jnc::rtl::DynamicLayout*)layout)->m_size;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicLayout_getBufferSize(jnc_DynamicLayout* layout) {
	return ((jnc::rtl::DynamicLayout*)layout)->m_bufferSize;
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DynamicLayout_getSizeLimit(jnc_DynamicLayout* layout) {
	return ((jnc::rtl::DynamicLayout*)layout)->m_sizeLimit;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_DynamicLayout_setSizeLimit(
	jnc_DynamicLayout* layout,
	size_t limit
) {
	((jnc::rtl::DynamicLayout*)layout)->m_sizeLimit = limit;
}

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_DynamicLayout_getMode(jnc_DynamicLayout* layout) {
	return ((jnc::rtl::DynamicLayout*)layout)->m_mode;
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_DynamicLayoutSink*
jnc_DynamicLayout_getSink(jnc_DynamicLayout* layout) {
	return (jnc_DynamicLayoutSink*)((jnc::rtl::DynamicLayout*)layout)->m_sink;
}

JNC_EXTERN_C
JNC_EXPORT_O
void
jnc_DynamicLayout_reset(
	jnc_DynamicLayout* layout,
	uint_t mode,
	jnc_DynamicLayoutSink* sink,
	jnc_DataPtr ptr,
	size_t size
) {
	((jnc::rtl::DynamicLayout*)layout)->reset(mode, (jnc::rtl::DynamicLayoutSink*)sink, ptr, size);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_DynamicLayout_closeAllGroups(jnc_DynamicLayout* layout) {
	return ((jnc::rtl::DynamicLayout*)layout)->closeAllGroups();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
