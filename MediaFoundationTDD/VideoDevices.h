#pragma once

#include <windows.h>
#include <string>
#include <atlcomcli.h>
#include <memory>

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
	VideoDevices(HWND windowForVideo);
	~VideoDevices();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFActivate>				GetCaptureVideoDevice(std::wstring videoDeviceName);
	CComPtr<IMFActivate>				GetVideoRenderer();

private:
	VideoDevices();
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<VideoDevicesRep> m_pRep = 0;
#pragma warning(pop)
};