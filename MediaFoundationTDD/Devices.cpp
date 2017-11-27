#include "Devices.h"
#include "MFUtils.h"
#include "MFAttrToStrHelper.h"
#include "MediaTypeUtils.h"

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
	void						DumpFormats(CComPtr<IMFActivate> device, WCHAR* devicename);

	IMFActivate**				mCaptureDevices			= NULL;
	unsigned int				mNumberOfCaptureDevices	= 0;
	CComPtr<IMFActivate>		mRenderer				= NULL;
	bool						m_DumpAtributes			= false;
};
Devices::Devices(CComPtr<IMFAttributes> attributesPtr, CComPtr<IMFActivate> renderer)
{
	m_pRep = std::make_unique<DevicesRep>(attributesPtr, renderer);
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

void DevicesRep::DumpFormats(CComPtr<IMFActivate> device, WCHAR* devicename)
{
	CComPtr<IMFMediaSource> mediaSource = nullptr;
	OnERR_return(device->ActivateObject(IID_IMFMediaSource, (void**)&mediaSource));
	CComPtr<IMFPresentationDescriptor>	presentationDescriptor = nullptr;
	OnERR_return(mediaSource->CreatePresentationDescriptor(&presentationDescriptor));
	DWORD presentationDescriptorCount = 0;
	OnERR_return(presentationDescriptor->GetStreamDescriptorCount(&presentationDescriptorCount));
	for (DWORD s = 0; s < presentationDescriptorCount; s++)
	{
		BOOL selected = FALSE;
		CComPtr<IMFStreamDescriptor> streamDescriptor = nullptr;
		OnERR_return(presentationDescriptor->GetStreamDescriptorByIndex(s, &selected, &streamDescriptor));

		// stream header
		wchar_t  mess[1024];
		swprintf_s(mess, 1024, L"%s stream(%d)\n", devicename, s);
		OutputDebugStringW(mess);

		CComPtr<IMFMediaTypeHandler> mediaTypeHandler = nullptr;
		OnERR_return(streamDescriptor->GetMediaTypeHandler(&mediaTypeHandler));
		DWORD mediaTypeCount = 0;
		OnERR_return(mediaTypeHandler->GetMediaTypeCount(&mediaTypeCount));
		for (DWORD m = 0; m < mediaTypeCount; m++)
		{
			CComPtr<IMFMediaType> mediaType = nullptr;
			OnERR_return(mediaTypeHandler->GetMediaTypeByIndex(m, &mediaType));
			UINT32 attributeCount = 0;
			OnERR_return(mediaType->GetCount(&attributeCount));

			// formats header
			swprintf_s(mess, 1024, L"%s stream(%d) format(%d) mode(%s)\n", devicename, s, m, MediaTypeUtils::ConvertCaptureInputModeToString(MediaTypeUtils::ConvertVideoMediaTypeToCaptureInputMode(mediaType)).c_str());
			OutputDebugStringW(mess);

			for (unsigned int a = 0; a < attributeCount; a++)
			{
				GUID guidKey;
				PROPVARIANT value;
				OnERR_return(mediaType->GetItemByIndex(a, &guidKey, &value));

				std::wstring attrName = MFAttrToStrHelper::GUIDToAttrName(guidKey);
				std::wstring attrVal = MFAttrToStrHelper::PropToValue(mediaType, guidKey, value);

				wchar_t  mess[1024];
				swprintf_s(mess, 1024, L"%s stream(%d) format(%d) ATTR %.3d %-56s %s\n", devicename, s, m, a, attrName.c_str(), attrVal.c_str());
				OutputDebugStringW(mess);
			}
		}
	}
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
				DumpFormats(mCaptureDevices[i], devicename);
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