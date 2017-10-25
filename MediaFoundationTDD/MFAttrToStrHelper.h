#include <windows.h>
#include <string>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

// so you can use a GUID as a key in STL
inline bool operator<(const GUID & left, const GUID & right)
{
	return (memcmp(&left, &right, sizeof(GUID)) > 0 ? true : false);
}
namespace MFAttrToStrHelper
{
	const std::wstring GUIDToAttrName(const GUID guid);
	std::wstring MediaTypeToValue(CComPtr<IMFActivate> device);
	std::wstring PropToVarType(PROPVARIANT value);
	std::wstring PropToValueImp(CComPtr<IMFAttributes> attributes, GUID guid, PROPVARIANT value);
	
	template <class T> std::wstring PropToValue(CComPtr<T> comType, GUID guid, PROPVARIANT value)
	{
		IMFAttributes* attributes;
		HRESULT hr = comType->QueryInterface(IID_PPV_ARGS(&attributes));
		if (SUCCEEDED(hr))
		{
			return PropToValueImp(attributes, guid, value);
		}
		else
		{
			OutputDebugStringW(L"PropToValue CALLED WITH non IMFAttributes COM OBJECT!!");
			return L"UNKNOWN";
		}
	}
}