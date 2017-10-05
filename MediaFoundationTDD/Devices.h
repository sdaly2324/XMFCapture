#pragma once
#include <windows.h>
#include <vector>

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
	Devices(IMFAttributes* attributesPtr);
	~Devices();

	HRESULT						GetLastHRESULT();

	IMFActivate**				GetDevices();
	unsigned int				GetNumDevices();
	std::vector<std::wstring>	GetDeviceNames();
	IMFActivate*				GetDeviceByName(std::wstring deviceName);

private:
	DevicesRep* m_pRep = 0;
};
