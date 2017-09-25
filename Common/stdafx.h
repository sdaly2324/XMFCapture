// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
//#ifndef WINVER				// Allow use of features specific to Windows XP or later.
//#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
//#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT _WIN32_WINNT_MAXVER	// Change this to the appropriate value to target other versions of Windows.
#endif						

//#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
//#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
//#endif
//
//#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
//#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
//#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#include <afxctl.h>         // MFC support for ActiveX Controls
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#include <atlbase.h>
#include <atlcom.h>
#include <afx.h>
#include <atlwin.h>

enum E_INPUTFORMAT
{
	FORMAT_TYPE_NONE = -1,
	FORMAT_TYPE_PAL = 1,
	FORMAT_TYPE_NTSC = 2
};

using namespace ATL;

#define LOG_HR_ERROR(hr) \
	if (FAILED(hr))\
{\
	LPTSTR errorText;\
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorText, 0, NULL);\
	wchar_t  ws[1024];\
	swprintf(ws, 1024, L"%hs", __FILE__);\
	WCHAR mess[256];\
	swprintf_s(mess, 256, L"HRESULT(0x%x) ERROR FOUND!!\n%sFile(%s) Line(%d)\n", hr, errorText, ws, __LINE__);\
	OutputDebugString(mess);\
}

#define BREAK_ON_NULL(value, newHr)     {if(value == NULL) { hr = newHr; break; }}

#include <string>
#include <comdef.h>

std::wstring MyStringConverter(char* stringToConvert);
std::string MyStringConverterW(WCHAR* stringToConvert);

void XOSHResult(HRESULT hr, std::wstring customMessage, char* function, char* file, int line);
bool XOSHResultb(HRESULT hr, char* function, char* file, int line);
void XOSHResultv(HRESULT hr, char* function, char* file, int line);

#define BREAK_ON_FAIL(hr)  { if(XOSHResultb(hr, __FUNCTION__, __FILE__, __LINE__) == false) break;}
#define RETURN_HR_ON_FAIL(hr)  { if(XOSHResultb(hr, __FUNCTION__, __FILE__, __LINE__) == false) return hr;}
#define BREAK_ON_NULL(value, newHr)     {if(value == NULL) { hr = newHr; break; }}
#define SUCCEEDED_Xb(hr) XOSHResultb(hr, __FUNCTION__, __FILE__, __LINE__)
#define SUCCEEDED_Xv(hr) XOSHResultv(hr, __FUNCTION__, __FILE__, __LINE__)