#include "IMFWrapper.h"

#include <string>
#include <comdef.h>

std::wstring ConvertCharToWString(char* stringToConvert)
{
	std::string sstr(stringToConvert);
	return std::wstring(sstr.begin(), sstr.end());
}
HRESULT IMFWrapper::HRESULTPrintIfErrAndSave(HRESULT hr, char* function, char* file, int line)
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
	return hr;
}

HRESULT IMFWrapper::GetLastHRESULT()
{
	return mLastHR;
}

void IMFWrapper::SetFAILED()
{
	mLastHR = E_FAIL;
}

bool IMFWrapper::LastHR_OK()
{
	return mLastHR == S_OK;
}
bool IMFWrapper::LastHR_FAIL()
{
	return mLastHR != S_OK;
}