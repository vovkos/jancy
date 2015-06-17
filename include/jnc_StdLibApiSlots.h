#pragma once

namespace jnc {

//.............................................................................

enum StdApiSlot
{	
	StdApiSlot_Error,
	StdApiSlot_String,
	StdApiSlot_StringRef,
	StdApiSlot_StringBuilder,
	StdApiSlot_StringHashTable,
	StdApiSlot_VariantHashTable,
	StdApiSlot_ListEntry,
	StdApiSlot_List,
	StdApiSlot_ConstBuffer,
	StdApiSlot_ConstBufferRef,
	StdApiSlot_BufferRef,
	StdApiSlot_Buffer,
	StdApiSlot_Library,
	StdApiSlot_Recognizer,
	
	StdApiSlot__Count,
};

//.............................................................................

} // namespace jnc {
