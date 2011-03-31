// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once
#pragma warning(disable: 4996 4353)

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200

#include <atlbase.h>
#include <atlapp.h>
#include <atltypes.h>
#define _WTL_USE_CSTRING
#include <atlmisc.h>
#include "atlrx.h"
#include <tchar.h>

#ifdef _UNICODE
#define tstring std::wstring
#define tifstream std::wifstream
#define tofstream std::wofstream
#else
#define tstring std::string
#define tifstream std::ifstream
#define tofstream std::ofstream
#endif

extern CAppModule _Module;

#include <atlwin.h>
#include <atlctrls.h>
#include <atlctrlx.h>

//#include "atlwfile.h"
#define	PEK_TX_TECHLEVEL 1
#include "textfile.h"


#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
