#pragma once

#include <windows.h>
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

////////////////////////
// HRESULT helpers
#ifdef XOSMFCAPTURECPP_EXPORTS
#define XOSMFCAPTURECPP_API __declspec(dllexport)
#else
#define XOSMFCAPTURECPP_API __declspec(dllimport)
#endif

void XOSMFCAPTURECPP_API XOSHResult(HRESULT hr, std::wstring customMessage, char* function, char* file, int line);
bool XOSMFCAPTURECPP_API XOSHResultb(HRESULT hr, char* function, char* file, int line);
void XOSMFCAPTURECPP_API XOSHResultv(HRESULT hr, char* function, char* file, int line);
#define SUCCEEDED_XOSb(hr) XOSHResultb(hr, __FUNCTION__, __FILE__, __LINE__)
#define SUCCEEDED_XOSv(hr) XOSHResultv(hr, __FUNCTION__, __FILE__, __LINE__)
#define DUMPHr_XOSv(hr, mess) XOSHResult(hr, mess, __FUNCTION__, __FILE__, __LINE__)

////////////////////////
// string helpers
std::wstring MyStringConverter(char* stringToConvert);
std::string MyStringConverter(WCHAR* stringToConvert);

////////////////////////
// IMFAttributes logger helpers
void XOSMFCAPTURECPP_API DumpAttrImp(CComPtr<IMFAttributes> attrs, const std::wstring& typeName, const std::wstring& name);
template <class T> void DumpAttr(CComPtr<T> pComType, const std::wstring& typeName, const std::wstring& name)
{
	if (pComType == NULL)
	{
		OutputDebugStringW(L"DumpAttr CALLED WITH NULL COM OBJECT!!");
		return;
	}

	IMFAttributes* attrs;
	HRESULT hr = pComType->QueryInterface(IID_PPV_ARGS(&attrs));
	if (SUCCEEDED_XOSb(hr))
	{
		DumpAttrImp(attrs, typeName, name);
	}
	else
	{
		OutputDebugStringW(L"DumpAttr CALLED WITH non IMFAttributes COM OBJECT!!");
	}
}

////////////////////////
// enums
enum XOSMediaType
{
	Video,
	Audio,
	UNKNOWN
};

////////////////////////
// const values
const GUID MEDIASUBTYPE_HDYC = { 0x43594448, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };