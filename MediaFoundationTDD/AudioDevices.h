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
class AudioDevicesRep;
class MediaFoundationTDD_API AudioDevices
{
public:
	AudioDevices();
	~AudioDevices();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFActivate>				GetAudioDevice(std::wstring audioDeviceName);

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<AudioDevicesRep> m_pRep = 0;
#pragma warning(pop)
};