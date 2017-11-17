#include "MFUtils.h"
#include "MFAttrToStrHelper.h"

#include "mfidl.h"
#include <comdef.h>

std::wstring ConvertCharToWString(char* stringToConvert)
{
	std::string sstr(stringToConvert);
	return std::wstring(sstr.begin(), sstr.end());
}

HRESULT MFUtils::mLastHR = S_OK;
MFUtils::MFUtils()
{

}

HRESULT MFUtils::HRESULTLogErr(HRESULT hr, char* function, char* file, int line)
{
	if (FAILED(hr))
	{
		WCHAR mess[1024];
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		swprintf_s(mess, 1024, L"hr(0x%x) err(%s) Function(%s) FILE(%s) LINE(%d)\n", hr, errMsg, ConvertCharToWString(function).c_str(), ConvertCharToWString(file).c_str(), line);
		OutputDebugStringW(mess);
	}
	mLastHR = hr;
	return mLastHR;
}
HRESULT MFUtils::GetLastHRESULT()
{
	return mLastHR;
}
void MFUtils::SetLastHR_Fail()
{
	mLastHR = E_FAIL;
}
bool MFUtils::LastHR_OK()
{
	return mLastHR == S_OK;
}
bool MFUtils::LastHR_FAIL()
{
	return mLastHR != S_OK;
}

void MFUtils::DumpAttrImp(CComPtr<IMFAttributes> attrs, const std::wstring& typeName, const std::wstring& name)
{
	UINT32 count = 0;
	OnERR_return(attrs->GetCount(&count));
	for (unsigned int x = 0; x < count; x++)
	{
		GUID guid;
		PROPVARIANT value;
		OnERR_return(attrs->GetItemByIndex(x, &guid, &value));
		std::wstring attrName = MFAttrToStrHelper::GUIDToAttrName(guid);
		std::wstring attrVal = L"UNKOWN";
		attrVal = MFAttrToStrHelper::PropToValue(attrs, guid, value);
		wchar_t  mess[1024];
		swprintf_s(mess, 1024, L"%s (%s) ATTR %.3d %-56s %s\n", typeName.c_str(), name.c_str(), x, attrName.c_str(), attrVal.c_str());
		OutputDebugStringW(mess);
	}
}