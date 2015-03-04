#pragma once

namespace jnc {

//.............................................................................

enum StdApiSlot
{	
	StdApiSlot_Error,
	StdApiSlot_String,
	StdApiSlot_StringRef,
	StdApiSlot_StringBuilder,
	StdApiSlot_ConstBuffer,
	StdApiSlot_ConstBufferRef,
	StdApiSlot_BufferRef,
	StdApiSlot_Buffer,
	StdApiSlot_Recognizer,
	
	StdApiSlot__Count,
};

//.............................................................................

} // namespace jnc {
