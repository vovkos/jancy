#pragma once

#include "MyWidget.h"
#include "MyLayout.h"
#include "MyLabel.h"
#include "MyButton.h"
#include "MyCheckBox.h"
#include "MyTextEdit.h"
#include "MySlider.h"

//.............................................................................

class MyLib: public jnc::StdLib
{
public:
	JNC_BEGIN_LIB ()
		JNC_STD_FUNCTION (jnc::StdFunction_Printf,  &stdPrintf)
		JNC_TYPE (MyWidget)
		JNC_TYPE (MyLayout)
		JNC_TYPE (MyLabel)
		JNC_TYPE (MyButton)
		JNC_TYPE (MyCheckBox)
		JNC_TYPE (MyTextEdit)
		JNC_TYPE (MySlider)
		JNC_LIB (jnc::StdLib)
	JNC_END_LIB ()

	static
	int
	stdPrintf (
		const char* format,
		...
		);
};

//.............................................................................
