// stdafx.cpp : source file that includes just the standard includes
// XOSCapture.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

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