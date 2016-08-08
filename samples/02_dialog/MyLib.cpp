#include "pch.h"
#include "MyLib.h"
#include "MyWidget.h"
#include "MyLayout.h"
#include "MyLabel.h"
#include "MyButton.h"
#include "MyCheckBox.h"
#include "MyTextEdit.h"
#include "MySlider.h"
#include "MainWindow.h"

//.............................................................................

int
stdPrintf (
	const char* format,
	...
	)
{
	va_list va;
	va_start (va, format);
	return getMainWindow ()->output_va (format, va);
}

//.............................................................................

JNC_DEFINE_LIB (
	MyLib,
	g_myLibGuid,
	"MyLib",
	"Sample extension library"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(MyLib)
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (MyLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (MyLayout)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (MyLabel)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (MyButton)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (MyCheckBox)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (MyTextEdit)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (MySlider)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

JNC_BEGIN_LIB_FUNCTION_MAP (MyLib)
	JNC_MAP_FUNCTION ("printf",  &stdPrintf)
	JNC_MAP_TYPE (MyWidget)
	JNC_MAP_TYPE (MyLayout)
	JNC_MAP_TYPE (MyLabel)
	JNC_MAP_TYPE (MyButton)
	JNC_MAP_TYPE (MyCheckBox)
	JNC_MAP_TYPE (MyTextEdit)
	JNC_MAP_TYPE (MySlider)
JNC_END_LIB_FUNCTION_MAP ()

//.............................................................................
