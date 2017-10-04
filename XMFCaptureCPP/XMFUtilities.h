#pragma once

#include <windows.h>
#include <memory>
#include <string>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>
#include <string>
#include <comdef.h>

////////////////////////
// inline helpers

// so you can use a GUID as a key in STL
inline bool operator<(const GUID & lhs, const GUID & rhs)
{
	return (memcmp(&lhs, &rhs, sizeof(GUID)) > 0 ? true : false);
}

#define DUMPHr_Xv(hr, mess) XHResult(hr, mess, __FUNCTION__, __FILE__, __LINE__)

enum XMediaType
{
	Video,
	Audio,
	UNKNOWN
};

////////////////////////
// const values
const GUID MEDIASUBTYPE_HDYC = { 0x43594448, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

#define XOSString std::shared_ptr<std::wstring>
#define XOSStringList std::vector<XOSString>
bool XOSStringsAreTheSame(XOSString left, XOSString right);

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

void DumpAttrImp(CComPtr<IMFAttributes> attrs, const std::wstring& typeName, const std::wstring& name);
template <class T> void DumpAttr(CComPtr<T> pComType, const std::wstring& typeName, const std::wstring& name)
{
	if (pComType == NULL)
	{
		OutputDebugStringW(L"DumpAttr CALLED WITH NULL COM OBJECT!!");
		return;
	}

	IMFAttributes* attrs;
	HRESULT hr = pComType->QueryInterface(IID_PPV_ARGS(&attrs));
	if (SUCCEEDED_Xb(hr))
	{
		DumpAttrImp(attrs, typeName, name);
	}
	else
	{
		OutputDebugStringW(L"DumpAttr CALLED WITH non IMFAttributes COM OBJECT!!");
	}
}