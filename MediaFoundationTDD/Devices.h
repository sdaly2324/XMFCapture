#pragma once
#include <windows.h>
#include <vector>
#include <atlcomcli.h>
#include <memory>

struct IMFActivate;
struct IMFAttributes;
class DevicesRep;
class Devices
{
public:
	Devices(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer);
	~Devices();

	HRESULT						GetLastHRESULT();

	CComPtr<IMFActivate>		GetCaptureDeviceByName(std::wstring deviceNameToFind);
	CComPtr<IMFActivate>		GetRenderer();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<DevicesRep> m_pRep = 0;
#pragma warning(pop)
};
