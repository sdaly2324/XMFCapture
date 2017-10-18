#include "AudioDevices.h"
#include "AttributesFactory.h"
#include "Devices.h"
#include "IMFWrapper.h"

#include <mfobjects.h>
#include <mfidl.h>

class AudioDevicesRep : public IMFWrapper , public Devices
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
		m_pRep = std::unique_ptr<AudioDevicesRep>(new AudioDevicesRep(attributesFactory.CreateAudioDeviceAttributes(), renderer));
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
	return m_pRep->IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFActivate> AudioDevices::GetCaptureAudioDevice(std::wstring audioDeviceName)
{
	return m_pRep->GetCaptureDeviceByName(audioDeviceName);
}

CComPtr<IMFActivate> AudioDevices::GetAudioRenderer()
{
	return m_pRep->GetRenderer();
}