#pragma once
#include <windows.h>
#include <vector>
#include <atlcomcli.h>
#include <memory>

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
	Devices(CComPtr<IMFAttributes> attributesPtr);
	~Devices();

	HRESULT						GetLastHRESULT();

	CComPtr<IMFActivate>		GetDeviceByName(std::wstring deviceName);

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<DevicesRep> m_pRep = 0;
#pragma warning(pop)
};
