#pragma once
#include <windows.h>

class IMFWrapper
{
public:
	HRESULT HRESULTLogErr(HRESULT hr, char* function, char* file, int line);
	HRESULT GetLastHRESULT();
	bool LastHR_OK();
	bool LastHR_FAIL();
	void SetLastHR_Fail();
private:
	HRESULT mLastHR = S_OK;
};
#define OnERR_return(hr) HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return;
#define OnERR_return_NULL(hr) HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return NULL;
#define OnERR_return_HR(hr) HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return hr;
#define OnERR_return_false(hr) HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return false;
#define IsHRError(hr) (HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__) != S_OK)