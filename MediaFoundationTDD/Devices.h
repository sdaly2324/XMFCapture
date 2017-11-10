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
