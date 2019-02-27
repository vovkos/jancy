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

#define _CRT_SECURE_NO_WARNINGS // disable useless warnings about "unsafe" string functions
#define _SCL_SECURE_NO_WARNINGS // disable useless warnings about "unsafe" iterators

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

// http://connect.microsoft.com/VisualStudio/feedback/details/621653/including-stdint-after-intsafe-generates-warnings
// warning C4005: 'INT8_MIN' : macro redefinition
#pragma warning(disable : 4005)
#include <stdint.h>
#include <intsafe.h>
#pragma warning(default : 4005)


#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define _CRT_SECURE_NO_WARNINGS // disable useless warnings about "unsafe" string functions

//..............................................................................

// LLVM

// warning C4146: unary minus operator applied to unsigned type, result still unsigned
// warning C4355: 'this' : used in base member initializer list
// warning C4800: 'unsigned int' : forcing value to bool 'true' or 'false' (performance warning)
// warning C4244: 'return' : conversion from 'uint64_t' to 'unsigned int', possible loss of data

#pragma warning(disable: 4146)
#pragma warning(disable: 4355)
#pragma warning(disable: 4800)
#pragma warning(disable: 4244)

#undef min
#undef max

#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Intrinsics.h"
#include "llvm/PassManager.h"
#include "llvm/IRBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/JITEventListener.h"

#include "../lib/MC/MCDisassembler/EDDisassembler.h"
#include "../lib/MC/MCDisassembler/EDInst.h"
#include "../lib/MC/MCDisassembler/EDOperand.h"
#include "../lib/MC/MCDisassembler/EDToken.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCSubtargetInfo.h"

#include "llvm/CodeGen/MachineCodeInfo.h"

#pragma warning(default: 4146)
#pragma warning(default: 4355)
#pragma warning(default: 4800)
#pragma warning(default: 4244)

//..............................................................................

// AXL

#include "axl_io_MappedFile.h"
#include "axl_rtl_ArrayList.h"
#include "axl_rtl_BitMap.h"
#include "axl_rtl_HandleTable.h"
#include "axl_rtl_StringHashTable.h"
#include "axl_rtl_EscapeEncoding.h"
#include "axl_io_FilePathUtils.h"
#include "axl_lex_RagelLexer.h"
#include "axl_err_ParseError.h"
#include "axl_jnc_Value.h"
#include "axl_jnc_StdLib.h"
#include "axl_jnc_Module.h"
#include "axl_jnc_Runtime.h"
#include "axl_jnc_Disassembler.h"
#include "../src/axl_jnc/axl_jnc_Parser.llk.h"


#include "axl_rtl_HexEncoding.h"

using namespace axl;

//..............................................................................

#include <new>
#include <typeinfo>

__declspec(selectany)
class CClearTypeInfoCache
{
public:
	CClearTypeInfoCache()
	{
		atexit(ClearTypeInfoCache);
	}

protected:
	static
	void
	ClearTypeInfoCache()
	{
	   __type_info_node* & node = __type_info_root_node._Next;
	   while(node)
	   {
		  if (node->_MemPtr)
		  {
			 delete node->_MemPtr;
		  }
		  __type_info_node* tempNode = node;
		  node = node->_Next;
		  delete tempNode;
	   }
	}
} g_ClearTypeInfoCache;
