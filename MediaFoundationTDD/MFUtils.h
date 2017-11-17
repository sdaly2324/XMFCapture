#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <string>

struct IMFAttributes;
class MFUtils
{
public:
	MFUtils();
	static HRESULT HRESULTLogErr(HRESULT hr, char* function, char* file, int line);
	static HRESULT GetLastHRESULT();
	static bool LastHR_OK();
	static bool LastHR_FAIL();
	static void SetLastHR_Fail();

	static void DumpAttrImp(CComPtr<IMFAttributes> attrs, const std::wstring& typeName, const std::wstring& name);
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
	static HRESULT mLastHR;
};
#define OnERR_return(hr)		MFUtils::HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return;
#define OnERR_return_NULL(hr)	MFUtils::HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return NULL;
#define OnERR_return_HR(hr)		MFUtils::HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return hr;
#define OnERR_return_false(hr)	MFUtils::HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__); if(!LastHR_OK()) return false;
#define IsHRError(hr)			(MFUtils::HRESULTLogErr(hr, __FUNCTION__, __FILE__, __LINE__) != S_OK)