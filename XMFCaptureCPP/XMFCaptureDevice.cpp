#include "stdafx.h"

#include "XMFCaptureDevice.h"
#include "XMFFormat.h"
#include "XMFAttrToStrHelper.h"

#include <mfobjects.h>
#include <mfapi.h>
#include <mfidl.h>

class XMFCaptureDeviceRep
{
public:
	XMFCaptureDeviceRep(CComPtr<IMFActivate> pMFActivate, GUID type);
	virtual ~XMFCaptureDeviceRep();

	WCHAR* GetDeviceName();

	bool SupportsAudio();
	bool SupportsVideo();
	bool SupportsAudioAndVideo();
	bool operator==(const XMFCaptureDeviceRep& obj);

	HRESULT GetIMFMediaSource(CComPtr<IMFMediaSource>& pMFMediaSource);
	HRESULT GetIMFActivate(CComPtr<IUnknown>& pIMFActivate);
	HRESULT StartCapture();
	HRESULT CheckDeviceLost(DEV_BROADCAST_HDR* pHdr, bool* pbDeviceLost);
	void StopCapture();

	// true if any one format in the list is a full match
	bool SupportsAnyOfTheseFormats(std::vector<std::shared_ptr<XMFFormat>> validFormats);

private:
	XMFCaptureDeviceRep();

	// int helpers
	HRESULT QueryDeviceAttributes();
	HRESULT QuerySourceAttributes();
	HRESULT QueryPresentationDescriptorAttributes();
	HRESULT QueryStreamDescriptorAttributes(CComPtr<IMFStreamDescriptor> pMFStreamDescriptor, DWORD streamID);
	HRESULT QueryMediaTypeHandlerAttributes(CComPtr<IMFMediaTypeHandler> pMFMediaTypeHandler, DWORD streamID);
	HRESULT QueryMediaTypeAttributes(CComPtr<IMFMediaType> pMFMediaType, std::shared_ptr<XMFFormat> p_NewFormat);

	void AddXMFMediaType(GUID mediaType, std::wstring mediaTypeName);
	std::shared_ptr<XMFFormat> AddXMFFormat(std::shared_ptr<XMFFormat> p_NewFormat);
	HRESULT SetConnectionString();

	// DEBUG
	bool						m_DumpAtributes;
	WCHAR*						m_pDevicename;

	// Data saved from queries
	std::vector<XMediaType>	m_SupportedMediaTypes;
	std::vector<std::shared_ptr<XMFFormat>>		m_Formats;

	// base MFInterfaces
	CComPtr<IMFActivate>		m_pMFActivate;
	GUID						m_DeviceType;
	CComPtr<IMFMediaSource>		m_pMFMediaSource;
	CComPtr<IMFPresentationDescriptor>	m_pMFPresentationDescriptor;
	DWORD						m_MFPresentationDescriptorCount;

	WCHAR*						m_pCaptureLinkString;
	GUID						m_pCaptureLinkUID;

	bool						m_Capturing;
	CComAutoCriticalSection		m_critsec;
};

XMFCaptureDevice::XMFCaptureDevice(CComPtr<IMFActivate> pMFActivate, GUID type)
{
	m_RepPtr = new XMFCaptureDeviceRep(pMFActivate, type);
}
XMFCaptureDeviceRep::XMFCaptureDeviceRep(CComPtr<IMFActivate> pMFActivate, GUID type) :
m_DumpAtributes(false),
m_pDevicename(NULL),
m_pMFActivate(pMFActivate),
m_DeviceType(type),
m_pMFMediaSource(NULL),
m_pMFPresentationDescriptor(NULL),
m_MFPresentationDescriptorCount(0),
m_pCaptureLinkString(NULL),
m_Capturing(false)
{
	if (type == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID)
	{
		m_pCaptureLinkUID = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK;
	}
	else if (type == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID)
	{
		m_pCaptureLinkUID = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK;
	}
	else
	{
		OutputDebugStringW(L"XMFCaptureDeviceRep UNKNOW DEVICE TYPE!\n");
	}

	m_SupportedMediaTypes.clear();
	m_Formats.clear();

	if (m_pMFActivate)
	{
		// init device name
		if (SUCCEEDED_Xb(m_pMFActivate->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &m_pDevicename, NULL)))
		{
			if (m_DumpAtributes)
			{
				// new device header
				wchar_t  mess[1024];
				swprintf_s(mess, 1024, L"\n%s\n", m_pDevicename);
				OutputDebugStringW(mess);
				DumpAttr(m_pMFActivate, m_pDevicename, L"");
			}
		}
	}

	// build format list
	SUCCEEDED_Xv(QueryDeviceAttributes());
}

XMFCaptureDevice::~XMFCaptureDevice()
{
	if (m_RepPtr)
	{
		delete m_RepPtr;
		m_RepPtr = NULL;
	}
}
XMFCaptureDeviceRep::~XMFCaptureDeviceRep()
{
	if (m_pDevicename != NULL)
	{
		CoTaskMemFree(m_pDevicename);
	}

	m_SupportedMediaTypes.clear();
}

void XMFCaptureDeviceRep::AddXMFMediaType(GUID mediaType, std::wstring mediaTypeName)
{
	if (mediaType == MFMediaType_Video)
	{
		if (std::find(m_SupportedMediaTypes.begin(), m_SupportedMediaTypes.end(), XMediaType::Video) == m_SupportedMediaTypes.end())
		{
			m_SupportedMediaTypes.push_back(XMediaType::Video);
		}
	}
	else if (mediaType == MFMediaType_Audio)
	{
		if (std::find(m_SupportedMediaTypes.begin(), m_SupportedMediaTypes.end(), XMediaType::Audio) == m_SupportedMediaTypes.end())
		{
			m_SupportedMediaTypes.push_back(XMediaType::Audio);
		}
	}
	else
	{
		wchar_t  mess[1024];
		swprintf_s(mess, 1024, L"Device \"%s\" supports an unknown media type \"%s\"\n", m_pDevicename, mediaTypeName.c_str());
		OutputDebugStringW(mess);
	}
}

HRESULT XMFCaptureDeviceRep::QueryMediaTypeAttributes(CComPtr<IMFMediaType> pMFMediaType, std::shared_ptr<XMFFormat> p_NewFormat)
{
	if (pMFMediaType == NULL || p_NewFormat == NULL)
	{
		return E_INVALIDARG;
	}

	UINT32 pcItems = 0;
	HRESULT hr = pMFMediaType->GetCount(&pcItems);
	if (SUCCEEDED_Xb(hr))
	{
		if (m_DumpAtributes)
		{
			// formats header
			wchar_t  mess[1024];
			swprintf_s(mess, 1024, L"%s stream(%d) format(%d)\n", m_pDevicename, p_NewFormat->GetStreamID(), p_NewFormat->GetFormatIndex());
			OutputDebugStringW(mess);
		}
		for (unsigned int x = 0; x < pcItems; x++)
		{
			GUID pguidKey;
			PROPVARIANT pValue;
			hr = pMFMediaType->GetItemByIndex(x, &pguidKey, &pValue);
			if (SUCCEEDED_Xb(hr))
			{
				p_NewFormat->AddAtribute(pguidKey, XMFAttrToStrHelper::PropToValue(pMFMediaType, pguidKey, pValue));
				if (pguidKey == MF_MT_MAJOR_TYPE)
				{
					AddXMFMediaType(*(pValue.puuid), XMFAttrToStrHelper::PropToValue(pMFMediaType, pguidKey, pValue));
				}

				if (m_DumpAtributes)
				{
					std::wstring attrName = XMFAttrToStrHelper::GUIDToAttrName(pguidKey);
					std::wstring attrVal = XMFAttrToStrHelper::PropToValue(pMFMediaType, pguidKey, pValue);

					wchar_t  mess[1024];
					swprintf_s(mess, 1024, L"%s stream(%d) format(%d) ATTR %.3d %-56s %s\n", m_pDevicename, p_NewFormat->GetStreamID(), p_NewFormat->GetFormatIndex(), x, attrName.c_str(), attrVal.c_str());
					OutputDebugStringW(mess);
				}
			}
		}
	}
	return hr;
}

std::shared_ptr<XMFFormat> XMFCaptureDeviceRep::AddXMFFormat(std::shared_ptr<XMFFormat> p_NewFormat)
{
	bool foundDupe = false;
	for (auto& format : m_Formats)
	{
		if (*format == *p_NewFormat)
		{
			foundDupe = true;
			break;
		}
	}
	if (foundDupe == false)
	{
		m_Formats.push_back(p_NewFormat);
		return p_NewFormat;
	}
	return NULL;
}
HRESULT XMFCaptureDeviceRep::QueryMediaTypeHandlerAttributes(CComPtr<IMFMediaTypeHandler> pMFMediaTypeHandler, DWORD streamID)
{
	if (pMFMediaTypeHandler == NULL)
	{
		return E_INVALIDARG;
	}

	DWORD mediaTypeCount = 0;
	HRESULT hr = pMFMediaTypeHandler->GetMediaTypeCount(&mediaTypeCount);

	for (DWORD x = 0; (x < mediaTypeCount) && SUCCEEDED_Xb(hr); x++)
	{
		CComPtr<IMFMediaType> pMFMediaType = NULL;
		hr = pMFMediaTypeHandler->GetMediaTypeByIndex(x, &pMFMediaType);
		if (SUCCEEDED_Xb(hr))
		{	
			hr = QueryMediaTypeAttributes(pMFMediaType, AddXMFFormat(std::make_shared<XMFFormat>(m_pMFActivate, streamID, x)));
		}
	}

	return hr;
}
HRESULT XMFCaptureDeviceRep::QueryStreamDescriptorAttributes(CComPtr<IMFStreamDescriptor> pMFStreamDescriptor, DWORD streamID)
{
	if (pMFStreamDescriptor == NULL)
	{
		return E_INVALIDARG;
	}

	CComPtr<IMFMediaTypeHandler> pMFMediaTypeHandler = NULL;
	HRESULT hr = pMFStreamDescriptor->GetMediaTypeHandler(&pMFMediaTypeHandler);
	if (SUCCEEDED_Xb(hr))
	{
		hr = QueryMediaTypeHandlerAttributes(pMFMediaTypeHandler, streamID);
	}

	return hr;
}
HRESULT XMFCaptureDeviceRep::QueryPresentationDescriptorAttributes()
{
	if (m_pMFPresentationDescriptor == NULL)
	{
		return E_UNEXPECTED;
	}

	m_MFPresentationDescriptorCount = 0;
	HRESULT hr = m_pMFPresentationDescriptor->GetStreamDescriptorCount(&m_MFPresentationDescriptorCount);

	for (DWORD i = 0; (i < m_MFPresentationDescriptorCount) && SUCCEEDED_Xb(hr); i++)
	{
		BOOL selected = FALSE;
		CComPtr<IMFStreamDescriptor> pMFStreamDescriptor = NULL;
		hr = m_pMFPresentationDescriptor->GetStreamDescriptorByIndex(i, &selected, &pMFStreamDescriptor);
		if (SUCCEEDED_Xb(hr))
		{
			if (m_DumpAtributes)
			{
				// // stream header
				wchar_t  mess[1024];
				swprintf_s(mess, 1024, L"%s stream(%d)\n", m_pDevicename, i);
				OutputDebugStringW(mess);
			}
			hr = QueryStreamDescriptorAttributes(pMFStreamDescriptor, i);
		}
	}

	return hr;
}

HRESULT XMFCaptureDeviceRep::QuerySourceAttributes()
{
	if (m_pMFMediaSource == NULL)
	{
		return E_UNEXPECTED;
	}

	HRESULT hr = m_pMFMediaSource->CreatePresentationDescriptor(&m_pMFPresentationDescriptor);
	if (SUCCEEDED_Xb(hr))
	{
		hr = QueryPresentationDescriptorAttributes();
	}

	return hr;
}
HRESULT XMFCaptureDeviceRep::QueryDeviceAttributes()
{
	if (m_pMFActivate == NULL)
	{
		return E_UNEXPECTED;
	}

	HRESULT hr = m_pMFActivate->ActivateObject(__uuidof(IMFMediaSource), (void**) &m_pMFMediaSource);

	if (SUCCEEDED_Xb(hr))
	{
		hr = QuerySourceAttributes();
	}

	return hr;
}

WCHAR* XMFCaptureDevice::GetDeviceName()
{
	if (m_RepPtr)
	{
		return m_RepPtr->GetDeviceName();
	}
	return NULL;
}
WCHAR* XMFCaptureDeviceRep::GetDeviceName()
{
	return m_pDevicename;
}

bool XMFCaptureDevice::SupportsAudio()
{
	if (m_RepPtr)
	{
		return m_RepPtr->SupportsAudio();
	}
	return false;
}
bool XMFCaptureDeviceRep::SupportsAudio()
{
	if (std::find(m_SupportedMediaTypes.begin(), m_SupportedMediaTypes.end(), XMediaType::Audio) != m_SupportedMediaTypes.end())
	{
		return true;
	}
	return false;
}

bool XMFCaptureDevice::SupportsVideo()
{
	if (m_RepPtr)
	{
		return m_RepPtr->SupportsVideo();
	}
	return false;
}
bool XMFCaptureDeviceRep::SupportsVideo()
{
	if (std::find(m_SupportedMediaTypes.begin(), m_SupportedMediaTypes.end(), XMediaType::Video) != m_SupportedMediaTypes.end())
	{
		return true;
	}
	return false;
}

bool XMFCaptureDevice::SupportsAudioAndVideo()
{
	if (m_RepPtr)
	{
		return m_RepPtr->SupportsAudioAndVideo();
	}
	return false;
}
bool XMFCaptureDeviceRep::SupportsAudioAndVideo()
{
	if (SupportsVideo() && SupportsAudio())
	{
		return true;
	}
	return false;
}

bool XMFCaptureDevice::operator==(const XMFCaptureDevice& obj)
{
	if (m_RepPtr)
	{
		return m_RepPtr->operator==(*(obj.m_RepPtr));
	}
	return false;
}
bool XMFCaptureDeviceRep::operator==(const XMFCaptureDeviceRep& obj)
{
	if (wcscmp(m_pDevicename, obj.m_pDevicename) == 0)
	{
		return true;
	}
	return false;
}

HRESULT XMFCaptureDevice::StartCapture()
{
	if (m_RepPtr)
	{
		return m_RepPtr->StartCapture();
	}
	return E_FAIL;
}
HRESULT XMFCaptureDeviceRep::StartCapture()
{
	HRESULT hr = m_pMFActivate->GetAllocatedString(m_pCaptureLinkUID, &m_pCaptureLinkString, NULL);
	m_Capturing = true;
	return hr;
}

void XMFCaptureDevice::StopCapture()
{
	if (m_RepPtr)
	{
		m_RepPtr->StopCapture();
	}
}
void XMFCaptureDeviceRep::StopCapture()
{
	CoTaskMemFree(m_pCaptureLinkString);
	m_Capturing = false;
	m_pCaptureLinkString = NULL;
}

HRESULT XMFCaptureDevice::CheckDeviceLost(DEV_BROADCAST_HDR* pHdr, bool* pbDeviceLost)
{
	if (m_RepPtr)
	{
		return m_RepPtr->CheckDeviceLost(pHdr, pbDeviceLost);
	}
	return E_FAIL;
}
HRESULT XMFCaptureDeviceRep::CheckDeviceLost(DEV_BROADCAST_HDR* pHdr, bool* pbDeviceLost)
{
	if (pbDeviceLost == NULL || pHdr == NULL)
	{
		return E_POINTER;
	}
	if (!m_Capturing || pHdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
	{
		return S_OK;
	}

	DEV_BROADCAST_DEVICEINTERFACE *pDi = NULL;
	*pbDeviceLost = FALSE;

	CComCritSecLock<CComCriticalSection> lock(m_critsec);

	// Compare the device name with the symbolic link.
	pDi = (DEV_BROADCAST_DEVICEINTERFACE*) pHdr;

	if (m_pCaptureLinkString)
	{
		if (_wcsicmp(m_pCaptureLinkString, pDi->dbcc_name) == 0)
		{
			*pbDeviceLost = TRUE;
		}
	}
	return S_OK;
}

HRESULT XMFCaptureDevice::GetIMFMediaSource(CComPtr<IMFMediaSource>& pMFMediaSource)
{
	if (m_RepPtr)
	{
		return m_RepPtr->GetIMFMediaSource(pMFMediaSource);
	}
	return E_FAIL;
}
HRESULT XMFCaptureDeviceRep::GetIMFMediaSource(CComPtr<IMFMediaSource>& pMFMediaSource)
{
	if (m_pMFActivate)
	{
		return m_pMFActivate->ActivateObject(__uuidof(IMFMediaSource), (void**) &pMFMediaSource);
	}
	return E_UNEXPECTED;
}

HRESULT XMFCaptureDevice::GetIMFActivate(CComPtr<IUnknown>& pIMFActivate)
{
	if (m_RepPtr)
	{
		return m_RepPtr->GetIMFActivate(pIMFActivate);
	}
	return E_FAIL;
}
HRESULT XMFCaptureDeviceRep::GetIMFActivate(CComPtr<IUnknown>& pIMFActivate)
{
	pIMFActivate = m_pMFActivate;
	return S_OK;
}

bool XMFCaptureDevice::SupportsAnyOfTheseFormats(std::vector<std::shared_ptr<XMFFormat>> validFormats)
{
	if (m_RepPtr)
	{
		return m_RepPtr->SupportsAnyOfTheseFormats(validFormats);
	}
	return false;
}
bool XMFCaptureDeviceRep::SupportsAnyOfTheseFormats(std::vector<std::shared_ptr<XMFFormat>> validFormats)
{
	for (auto& aFormatWeAreLookingFor : validFormats)
	{
		for (auto& aSupporttedFormat : m_Formats)
		{
			if (aSupporttedFormat->CantainsAllAttributes(*aFormatWeAreLookingFor))
			{
				return true;
			}
		}
	}
	return false;
}