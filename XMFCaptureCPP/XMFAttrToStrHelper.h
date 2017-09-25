#include <windows.h>
#include <string>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

namespace XMFAttrToStrHelper
{
	const std::wstring GUIDToAttrName(const GUID pguidKey);
	std::wstring PropToVarType(PROPVARIANT pValue);

	std::wstring PropToValueImp(CComPtr<IMFAttributes> pAttributes, GUID pguidKey, PROPVARIANT pValue);
	template <class T> std::wstring PropToValue(CComPtr<T> pComType, GUID pguidKey, PROPVARIANT pValue)
	{
		IMFAttributes* attrs;
		HRESULT hr = pComType->QueryInterface(IID_PPV_ARGS(&attrs));
		if (SUCCEEDED_Xb(hr))
		{
			return PropToValueImp(attrs, pguidKey, pValue);
		}
		else
		{
			OutputDebugStringW(L"PropToValue CALLED WITH non IMFAttributes COM OBJECT!!");
			return L"UNKNOWN";
		}
	}
	//std::wstring PropToValue(CComPtr<IMFAttributes> pAttributes, GUID pguidKey, PROPVARIANT pValue);

	std::wstring MediaTypeToValue(CComPtr<IMFActivate> device);
}