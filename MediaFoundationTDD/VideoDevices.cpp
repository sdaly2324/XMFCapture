#include "VideoDevices.h"
#include "AttributesFactory.h"
#include "Devices.h"

#include <mfobjects.h>

class VideoDevicesRep : public Devices
{
public:
	VideoDevicesRep(CComPtr<IMFAttributes> attributesPtr);
	~VideoDevicesRep();

private:
};

VideoDevices::VideoDevices()
{
	AttributesFactory attributesFactory;
	m_pRep = std::unique_ptr<VideoDevicesRep>(new VideoDevicesRep(attributesFactory.CreateVideoDeviceAttributes()));
}
VideoDevicesRep::VideoDevicesRep(CComPtr<IMFAttributes> attributesPtr) :Devices(attributesPtr)
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

CComPtr<IMFActivate> VideoDevices::GetVideoDevice(std::wstring videoDeviceName)
{
	return m_pRep->GetDeviceByName(videoDeviceName);
}