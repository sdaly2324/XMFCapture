#pragma once

#include <windows.h>
#include <string>
#include <atlcomcli.h>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFActivate;
class VideoDevicesRep;
class MediaFoundationTDD_API VideoDevices
{
public:
	VideoDevices();
	~VideoDevices();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFActivate>				GetVideoDevice(std::wstring videoDeviceName);

private:
	VideoDevicesRep* m_pRep = 0;
};