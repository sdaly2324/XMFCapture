#include "Devices.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <mfidl.h>

class DevicesRep : public IMFWrapper
{
public:
	DevicesRep(CComPtr<IMFAttributes> attributesPtr);
	~DevicesRep();

	HRESULT						GetLastHRESULT();

	CComPtr<IMFActivate>		GetDeviceByName(std::wstring deviceName);

private:
	std::vector<std::wstring>	GetDeviceNames();

	IMFActivate**				mDevicesPtr			= NULL;
	unsigned int				mNumberOfDevices	= 0;
};
Devices::Devices(CComPtr<IMFAttributes> attributesPtr)
{
	m_pRep = std::unique_ptr<DevicesRep>(new DevicesRep(attributesPtr));
}
DevicesRep::DevicesRep(CComPtr<IMFAttributes> attributesPtr)
{
	OnERR_return(MFEnumDeviceSources(attributesPtr, &mDevicesPtr, &mNumberOfDevices));
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

std::vector<std::wstring> DevicesRep::GetDeviceNames()
{
	std::vector<std::wstring> retVal;
	for (UINT32 i = 0; i < mNumberOfDevices && LastHR_OK(); i++)
	{
		WCHAR* devicename;
		if (!IsHRError(mDevicesPtr[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &devicename, NULL)))
		{
			retVal.push_back(devicename);
		}
	}
	return retVal;
}

CComPtr<IMFActivate> Devices::GetDeviceByName(std::wstring deviceName)
{
	return m_pRep->GetDeviceByName(deviceName);
}
CComPtr<IMFActivate> DevicesRep::GetDeviceByName(std::wstring deviceName)
{
	CComPtr<IMFActivate> retVal = NULL;
	std::vector<std::wstring> myDeviceNames = GetDeviceNames();
	auto found = std::find(myDeviceNames.begin(), myDeviceNames.end(), deviceName);
	if (found != myDeviceNames.end())
	{
		int x = found - myDeviceNames.begin();
		retVal = mDevicesPtr[x];
	}
	return retVal;
}