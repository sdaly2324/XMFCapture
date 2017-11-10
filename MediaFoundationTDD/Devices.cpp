#include "Devices.h"
#include "MFUtils.h"

#include <mfapi.h>
#include <mfidl.h>

class DevicesRep : public MFUtils
{
public:
	DevicesRep(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer);
	~DevicesRep();

	HRESULT						GetLastHRESULT();

	CComPtr<IMFActivate>		GetCaptureDeviceByName(std::wstring deviceNameToFind);
	CComPtr<IMFActivate>		GetRenderer();

private:
	std::vector<std::wstring>	GetCaptureDeviceNames();

	IMFActivate**				mCaptureDevices			= NULL;
	unsigned int				mNumberOfCaptureDevices	= 0;
	CComPtr<IMFActivate>		mRenderer				= NULL;
	bool						m_DumpAtributes			= true;
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
	return MFUtils::GetLastHRESULT();
}

std::vector<std::wstring> DevicesRep::GetCaptureDeviceNames()
{
	std::vector<std::wstring> retVal;
	for (UINT32 i = 0; i < mNumberOfCaptureDevices && LastHR_OK(); i++)
	{
		WCHAR* devicename;
		if (!IsHRError(mCaptureDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &devicename, NULL)))
		{
			if (m_DumpAtributes)
			{
				// new device header
				wchar_t  mess[1024];
				swprintf_s(mess, 1024, L"\n%s\n", devicename);
				OutputDebugStringW(mess);
				CComPtr<IMFAttributes> deviceAttrs = mCaptureDevices[i];
				DumpAttr(deviceAttrs, devicename, L"");
			}
			retVal.push_back(devicename);
		}
	}
	return retVal;
}

CComPtr<IMFActivate> Devices::GetCaptureDeviceByName(std::wstring deviceNameToFind)
{
	return m_pRep->GetCaptureDeviceByName(deviceNameToFind);
}
CComPtr<IMFActivate> DevicesRep::GetCaptureDeviceByName(std::wstring deviceNameToFind)
{
	CComPtr<IMFActivate> retVal = NULL;
	std::vector<std::wstring> myDeviceNames = GetCaptureDeviceNames();
	int counter = 0;
	for each (auto deviceName in myDeviceNames)
	{
		if (deviceName.find(deviceNameToFind) != std::string::npos)
		{
			retVal = mCaptureDevices[counter];
		}
		counter++;
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