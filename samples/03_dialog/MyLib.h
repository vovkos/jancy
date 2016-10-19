#pragma once

//..............................................................................

// {D87388FE-05FD-4865-81A6-02A68CE5922D}
JNC_DEFINE_GUID (
	g_myLibGuid,
	0xd87388fe, 0x5fd, 0x4865, 0x81, 0xa6, 0x2, 0xa6, 0x8c, 0xe5, 0x92, 0x2d
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum MyLibCacheSlot
{
	MyLibCacheSlot_Widget,
	MyLibCacheSlot_Layout,
	MyLibCacheSlot_Label,
	MyLibCacheSlot_Button,
	MyLibCacheSlot_CheckBox,
	MyLibCacheSlot_TextEdit,
	MyLibCacheSlot_Slider,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DECLARE_LIB (MyLib)

//..............................................................................
