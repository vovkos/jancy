#pragma once

//.............................................................................

// {D87388FE-05FD-4865-81A6-02A68CE5922D}
AXL_SL_DEFINE_GUID (
	g_myLibGuid,
	0xd87388fe, 0x5fd, 0x4865, 0x81, 0xa6, 0x2, 0xa6, 0x8c, 0xe5, 0x92, 0x2d
	);

AXL_SELECT_ANY size_t g_myLibCacheSlot;

//.............................................................................

enum MyLibTypeCacheSlot
{
	MyLibTypeCacheSlot_Widget,
	MyLibTypeCacheSlot_Layout,
	MyLibTypeCacheSlot_Label,
	MyLibTypeCacheSlot_Button,
	MyLibTypeCacheSlot_CheckBox,
	MyLibTypeCacheSlot_TextEdit,
	MyLibTypeCacheSlot_Slider,
};

//.............................................................................
