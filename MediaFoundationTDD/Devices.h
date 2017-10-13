#pragma once
#include <windows.h>
#include <vector>
#include <atlcomcli.h>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFActivate;
struct IMFAttributes;
class DevicesRep;
class MediaFoundationTDD_API Devices
{
public:
	Devices(CComPtr<IMFAttributes> attributesPtr);
	~Devices();

	HRESULT						GetLastHRESULT();

	CComPtr<IMFActivate>		GetDeviceByName(std::wstring deviceName);

private:
	DevicesRep* m_pRep = 0;
};
