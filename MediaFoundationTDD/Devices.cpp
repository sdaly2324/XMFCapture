#include "Devices.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <mfidl.h>

class DevicesRep : public IMFWrapper
{
public:
	DevicesRep(IMFAttributes* attributesPtr);
	~DevicesRep();

	HRESULT						GetLastHRESULT();

	IMFActivate**				GetDevices();
	unsigned int				GetNumDevices();
	std::vector<std::wstring>	GetDeviceNames();
	IMFActivate*				GetDeviceByName(std::wstring deviceName);

private:
	IMFActivate**	mDevicesPtr			= NULL;
	unsigned int	mNumberOfDevices	= 0;
};
Devices::Devices(IMFAttributes* attributesPtr)
{
	m_pRep = new DevicesRep(attributesPtr);
}
DevicesRep::DevicesRep(IMFAttributes* attributesPtr)
{
	PrintIfErrAndSave(MFEnumDeviceSources(attributesPtr, &mDevicesPtr, &mNumberOfDevices));
}
Devices::~Devices()
{
	delete m_pRep;
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

IMFActivate** Devices::GetDevices()
{
	return m_pRep->GetDevices();
}
IMFActivate** DevicesRep::GetDevices()
{
	return mDevicesPtr;
}

unsigned int Devices::GetNumDevices()
{
	return m_pRep->GetNumDevices();
}
unsigned int DevicesRep::GetNumDevices()
{
	return mNumberOfDevices;
}

std::vector<std::wstring> Devices::GetDeviceNames()
{
	return m_pRep->GetDeviceNames();
}
std::vector<std::wstring> DevicesRep::GetDeviceNames()
{
	std::vector<std::wstring> retVal;
	for (UINT32 i = 0; i < mNumberOfDevices && LastHR_OK(); i++)
	{
		WCHAR* devicename;
		PrintIfErrAndSave(mDevicesPtr[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &devicename, NULL));
		if (LastHR_OK())
		{
			retVal.push_back(devicename);
		}
	}
	return retVal;
}

IMFActivate* Devices::GetDeviceByName(std::wstring deviceName)
{
	return m_pRep->GetDeviceByName(deviceName);
}
IMFActivate* DevicesRep::GetDeviceByName(std::wstring deviceName)
{
	IMFActivate* retVal = NULL;
	std::vector<std::wstring> myDeviceNames = GetDeviceNames();
	auto found = std::find(myDeviceNames.begin(), myDeviceNames.end(), deviceName);
	if (found != myDeviceNames.end())
	{
		int x = found - myDeviceNames.begin();
		retVal = mDevicesPtr[x];
	}
	return retVal;
}