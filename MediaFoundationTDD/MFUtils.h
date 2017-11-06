#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <string>

struct IMFAttributes;
class MFUtils
{
public:
	MFUtils();
	HRESULT HRESULTLogErr(HRESULT hr, char* function, char* file, int line);
	HRESULT GetLastHRESULT();
	bool LastHR_OK();
	bool LastHR_FAIL();
	void SetLastHR_Fail();

	void DumpAttrImp(CComPtr<IMFAttributes> attrs, const std::wstring& typeName, const std::wstring& name);
	template <class T> void DumpAttr(CComPtr<T> pComType, const std::wstring& typeName, const std::wstring& name)
	{
		if (pComType == NULL)
		{
			OutputDebugStringW(L"DumpAttr CALLED WITH NULL COM OBJECT!!\n");
			return;
		}

		IMFAttributes* attrs;
		HRESULT hr = pComType->QueryInterface(IID_PPV_ARGS(&attrs));
		if (SUCCEEDED(hr))
		{
			DumpAttrImp(attrs, typeName, name);
		}
		else
		{
			OutputDebugStringW(L"DumpAttr CALLED WITH non IMFAttributes COM OBJECT!!\n");
		}
	}
private:
	HRESULT mLastHR;
};
#define OnERR_return(hr)		HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return;
#define OnERR_return_NULL(hr)	HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return NULL;
#define OnERR_return_HR(hr)		HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return hr;
#define OnERR_return_false(hr)	HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return false;
#define IsHRError(hr)			(HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__) != S_OK)