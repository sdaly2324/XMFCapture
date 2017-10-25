#include "VideoDevices.h"
#include "AttributesFactory.h"
#include "Devices.h"

#include <mfobjects.h>
#include <mfidl.h>

class VideoDevicesRep : public Devices
{
public:
	VideoDevicesRep(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer);
	~VideoDevicesRep();

private:
};

VideoDevices::VideoDevices(HWND windowForVideo)
{
	CComPtr<IMFActivate> renderer = NULL;
	HRESULT hr = MFCreateVideoRendererActivate(windowForVideo, &renderer);
	if (SUCCEEDED(hr))
	{
		AttributesFactory attributesFactory;
		m_pRep = std::unique_ptr<VideoDevicesRep>(new VideoDevicesRep(attributesFactory.CreateVideoDeviceAttrs(), renderer));
	}
}
VideoDevicesRep::VideoDevicesRep(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer) : Devices(attributesPtr, renderer)
{
}
VideoDevices::~VideoDevices()
{
}
VideoDevicesRep::~VideoDevicesRep()
{
}

HRESULT VideoDevices::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}

CComPtr<IMFActivate> VideoDevices::GetCaptureVideoDevice(std::wstring videoDeviceName)
{
	return m_pRep->GetCaptureDeviceByName(videoDeviceName);
}

CComPtr<IMFActivate> VideoDevices::GetVideoRenderer()
{
	return m_pRep->GetRenderer();
}