#include "XMFUtilities.h"
#include "XMFAttrToStrHelper.h"

#include <mfapi.h>
#include <mfidl.h>

void DumpAttrImp(CComPtr<IMFAttributes> attrs, const std::wstring& typeName, const std::wstring& name)
{
	UINT32 pcItems = 0;
	HRESULT hr = attrs->GetCount(&pcItems);
	if (SUCCEEDED_Xb(hr))
	{
		for (unsigned int x = 0; x < pcItems; x++)
		{
			GUID pguidKey;
			PROPVARIANT pValue;
			hr = attrs->GetItemByIndex(x, &pguidKey, &pValue);
			if (SUCCEEDED_Xb(hr))
			{
				// get attribute name
				std::wstring attrName = XMFAttrToStrHelper::GUIDToAttrName(pguidKey);
				std::wstring attrVal = L"UNKOWN";

				attrVal = XMFAttrToStrHelper::PropToValue(attrs, pguidKey, pValue);

				wchar_t  mess[1024];
				swprintf_s(mess, 1024, L"%s (%s) ATTR %.3d %-56s %s\n", typeName.c_str(), name.c_str(), x, attrName.c_str(), attrVal.c_str());
				OutputDebugStringW(mess);
			}
		}
	}
}

bool XOSStringsAreTheSame(XOSString left, XOSString right)
{
	return *left == *right;
}
std::wstring MyStringConverter(char* stringToConvert)
{
	std::string sstr(stringToConvert);
	return std::wstring(sstr.begin(), sstr.end());
}
std::string MyStringConverterW(WCHAR* stringToConvert)
{
	std::wstring widestr(stringToConvert);
	return std::string(widestr.begin(), widestr.end());
}
void XOSHResult(HRESULT hr, std::wstring customMessage, char* function, char* file, int line)
{
	WCHAR mess[1024];
	_com_error err(hr);
	LPCTSTR errMsg = err.ErrorMessage();
	swprintf_s(mess, 1024, L"%s - hr(0x%x) err(%s) Function(%s) FILE(%s) LINE(%d)\n", customMessage.c_str(), hr, errMsg, MyStringConverter(function).c_str(), MyStringConverter(file).c_str(), line);
	OutputDebugStringW(mess);
}
void XOSHResultv(HRESULT hr, char* function, char* file, int line)
{
	if (FAILED(hr))
	{
		XOSHResult(hr, L"HRESULT FAILURE ", function, file, line);
	}
}
bool XOSHResultb(HRESULT hr, char* function, char* file, int line)
{
	if (FAILED(hr))
	{
		XOSHResult(hr, L"HRESULT FAILURE ", function, file, line);
		return false;
	}
	return true;
}