#include "AudioDevices.h"
#include "AttributesFactory.h"
#include "Devices.h"
#include "MFUtils.h"

#include <mfobjects.h>
#include <mfidl.h>

class AudioDevicesRep : public MFUtils , public Devices
{
public:
	AudioDevicesRep(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer);
	~AudioDevicesRep();

private:
};

AudioDevices::AudioDevices()
{
	CComPtr<IMFActivate> renderer = NULL;
	HRESULT hr = MFCreateAudioRendererActivate(&renderer);
	if (SUCCEEDED(hr))
	{
		AttributesFactory attributesFactory;
		m_pRep = std::make_unique<AudioDevicesRep>(attributesFactory.CreateAudioDeviceAttrs(), renderer);
	}
}
AudioDevicesRep::AudioDevicesRep(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer) : Devices(attributesPtr, renderer)
{
}
AudioDevices::~AudioDevices()
{
}
AudioDevicesRep::~AudioDevicesRep()
{
}

HRESULT AudioDevices::GetLastHRESULT()
{
	return m_pRep->MFUtils::GetLastHRESULT();
}

CComPtr<IMFActivate> AudioDevices::GetCaptureAudioDevice(std::wstring audioDeviceName)
{
	return m_pRep->GetCaptureDeviceByName(audioDeviceName);
}

CComPtr<IMFActivate> AudioDevices::GetAudioRenderer()
{
	return m_pRep->GetRenderer();
}