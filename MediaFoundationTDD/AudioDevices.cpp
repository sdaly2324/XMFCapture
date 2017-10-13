#include "AudioDevices.h"
#include "AttributesFactory.h"
#include "Devices.h"

#include <mfobjects.h>

class AudioDevicesRep : public Devices
{
public:
	AudioDevicesRep(CComPtr<IMFAttributes> attributesPtr);
	~AudioDevicesRep();

private:
};

AudioDevices::AudioDevices()
{
	AttributesFactory attributesFactory;
	m_pRep = new AudioDevicesRep(attributesFactory.CreateAudioDeviceAttributes());
}
AudioDevicesRep::AudioDevicesRep(CComPtr<IMFAttributes> attributesPtr):Devices(attributesPtr)
{
}
AudioDevices::~AudioDevices()
{
	delete m_pRep;
}
AudioDevicesRep::~AudioDevicesRep()
{
}

HRESULT AudioDevices::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}

CComPtr<IMFActivate> AudioDevices::GetAudioDevice(std::wstring audioDeviceName)
{
	return m_pRep->GetDeviceByName(audioDeviceName);
}