#pragma once

#include <windows.h>
#include <string>
#include <atlcomcli.h>
#include <memory>

struct IMFActivate;
class AudioDevicesRep;
class AudioDevices
{
public:
	AudioDevices();
	~AudioDevices();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFActivate>				GetCaptureAudioDevice(std::wstring audioDeviceName);
	CComPtr<IMFActivate>				GetAudioRenderer();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<AudioDevicesRep> m_pRep = 0;
#pragma warning(pop)
};