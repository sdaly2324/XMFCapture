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

////////////////////////
// IMFAttributes logger helpers
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

////////////////////////
// enums
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