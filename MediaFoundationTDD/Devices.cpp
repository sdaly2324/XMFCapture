#include "Devices.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <mfidl.h>

class DevicesRep : public IMFWrapper
{
public:
	DevicesRep(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer);
	~DevicesRep();

	HRESULT						GetLastHRESULT();

	CComPtr<IMFActivate>		GetCaptureDeviceByName(std::wstring deviceName);
	CComPtr<IMFActivate>		GetRenderer();

private:
	std::vector<std::wstring>	GetCaptureDeviceNames();

	IMFActivate**				mCaptureDevices			= NULL;
	unsigned int				mNumberOfCaptureDevices	= 0;
	CComPtr<IMFActivate>		mRenderer				= NULL;
};
Devices::Devices(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer)
{
	m_pRep = std::unique_ptr<DevicesRep>(new DevicesRep(attributesPtr, renderer));
}
DevicesRep::DevicesRep(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer)
{
	mRenderer = renderer;
	OnERR_return(MFEnumDeviceSources(attributesPtr, &mCaptureDevices, &mNumberOfCaptureDevices));
}
Devices::~Devices()
{
}
DevicesRep::~DevicesRep()
{
}

HRESULT Devices::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT DevicesRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}

std::vector<std::wstring> DevicesRep::GetCaptureDeviceNames()
{
	std::vector<std::wstring> retVal;
	for (UINT32 i = 0; i < mNumberOfCaptureDevices && LastHR_OK(); i++)
	{
		WCHAR* devicename;
		if (!IsHRError(mCaptureDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &devicename, NULL)))
		{
			retVal.push_back(devicename);
		}
	}
	return retVal;
}

CComPtr<IMFActivate> Devices::GetCaptureDeviceByName(std::wstring deviceName)
{
	return m_pRep->GetCaptureDeviceByName(deviceName);
}
CComPtr<IMFActivate> DevicesRep::GetCaptureDeviceByName(std::wstring deviceName)
{
	CComPtr<IMFActivate> retVal = NULL;
	std::vector<std::wstring> myDeviceNames = GetCaptureDeviceNames();
	auto found = std::find(myDeviceNames.begin(), myDeviceNames.end(), deviceName);
	if (found != myDeviceNames.end())
	{
		int x = found - myDeviceNames.begin();
		retVal = mCaptureDevices[x];
	}
	return retVal;
}

CComPtr<IMFActivate> Devices::GetRenderer()
{
	return m_pRep->GetRenderer();
}
CComPtr<IMFActivate> DevicesRep::GetRenderer()
{
	return mRenderer;
}