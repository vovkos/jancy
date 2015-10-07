#pragma once

#include "MyWidget.h"
#include "MyLayout.h"
#include "MyLabel.h"
#include "MyButton.h"
#include "MyCheckBox.h"
#include "MyTextEdit.h"
#include "MySlider.h"

//.............................................................................

class MyLib: public jnc::ext::ExtensionLib
{
public:
	JNC_BEGIN_LIB_MAP ()
		JNC_MAP_FUNCTION ("printf",  &stdPrintf)
		JNC_MAP_TYPE (MyWidget)
		JNC_MAP_TYPE (MyLayout)
		JNC_MAP_TYPE (MyLabel)
		JNC_MAP_TYPE (MyButton)
		JNC_MAP_TYPE (MyCheckBox)
		JNC_MAP_TYPE (MyTextEdit)
		JNC_MAP_TYPE (MySlider)
	JNC_END_LIB_MAP ()

	static
	int
	stdPrintf (
		const char* format,
		...
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

MyLib*
getMyLib (jnc::ext::ExtensionLibHost* host);

//.............................................................................
