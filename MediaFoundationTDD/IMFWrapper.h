#pragma once
#include <windows.h>

class IMFWrapper
{
public:
	HRESULT HRESULTPrintIfErrAndSave(HRESULT hr, char* function, char* file, int line);
	HRESULT GetLastHRESULT();
	bool LastHR_OK();
	bool LastHR_FAIL();
	void SetFAILED();
private:
	HRESULT mLastHR = S_OK;
};
#define PrintIfErrAndSave(hr) HRESULTPrintIfErrAndSave(hr, __FUNCTION__, __FILE__, __LINE__)