#include "stdafx.h"

#include "XMFCaptureAPI.h"
#include "XMFPreview.h"
#include "XMFCaptureDeviceManager.h"
#include "XMFCaptureDevice.h"
#include "XMFCaptureEngine.h"

#include <wmcodecdsp.h>

static const bool logBeginAPICall = false;
void LogBeginAPICall(std::string methodName)
{
	if (logBeginAPICall)
	{
		char mess[255];
		sprintf_s(mess, 255, "POOP Begin %s called\n", methodName.c_str());
		OutputDebugStringA(mess);
	}
}

static XMFCaptureAPI* gInstance = NULL;
class XMFCaptureAPIRep
{
public:
	XMFCaptureAPIRep();
	virtual ~XMFCaptureAPIRep();

	HRESULT			ReEnumerateDevices();

	XOSStringList	GetDevicePairNamesList();
	void			SetCurrentDevice(XOSString deviceName);
	XOSString		GetCurrentDevice();

	void			SetOutputPath(XOSString outputPath);

	bool			IsCapturing();
	HRESULT			CheckDeviceLost(DEV_BROADCAST_HDR* pHdr, bool* pbDeviceLost);

	HRESULT			StartCapture(bool useOld);
	HRESULT			StopCapture();

	HRESULT			StartPreview(HWND hwnd);
	HRESULT			StopPreview();
	HRESULT			IsPreviewRunning();

	HRESULT			get_FramesCaptured(unsigned long* pVal);
	HRESULT			get_FPSForCapture(long* pVal);
	HRESULT			get_PreviewIsOn(bool* pVal);
	void			SetVideoDisplaySize(long width, long height);

private:
	HRESULT GetMFDXBufferSource(CComPtr<IUnknown>& pVideoDevice);

	XOSString									m_OutputPath;
	bool										m_IsCapturing;
	bool										m_IsPreviewRunning;
	XOSString									m_CurrentDeviceName;
	std::shared_ptr<XMFCaptureDeviceManager>	m_pXMFCaptureDeviceManager;
	std::shared_ptr<XMFCaptureDevice>			m_CurrentVideoDevice;
	std::shared_ptr<XMFCaptureDevice>			m_CurrentAudioDevice;
	std::shared_ptr<XMFCaptureEngine>			m_XMFCaptureEngine;
	std::shared_ptr<XMFPreview>					m_XMFPreview;
};

XMFCaptureAPI* XMFCaptureAPI::GetInstance()
{
	LogBeginAPICall(__FUNCTION__);

	// Can only have one at a time!
	if (gInstance != NULL)
		return gInstance;

	gInstance = new XMFCaptureAPI();
	return gInstance;
}

XMFCaptureAPI::XMFCaptureAPI() : m_pRep(0)
{
	LogBeginAPICall(__FUNCTION__);

	m_pRep = new XMFCaptureAPIRep();
}
XMFCaptureAPIRep::XMFCaptureAPIRep() :
m_pXMFCaptureDeviceManager(NULL),
m_CurrentVideoDevice(NULL),
m_CurrentAudioDevice(NULL),
m_IsCapturing(false),
m_IsPreviewRunning(false)
{
	XOSString empty(new std::wstring(L""));
	m_CurrentDeviceName = empty;
	m_OutputPath = empty;
	m_pXMFCaptureDeviceManager = std::make_shared<XMFCaptureDeviceManager>();
	m_pXMFCaptureDeviceManager->ReEnumerateDevices();

	SUCCEEDED_Xv(MFStartup(MF_VERSION));
}

XMFCaptureAPI::~XMFCaptureAPI()
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		delete m_pRep;
		m_pRep = NULL;
	}

	SUCCEEDED_Xv(MFShutdown());
}
XMFCaptureAPIRep::~XMFCaptureAPIRep()
{
}

HRESULT XMFCaptureAPI::CheckDeviceLost(DEV_BROADCAST_HDR* pHdr, bool* pbDeviceLost)
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->CheckDeviceLost(pHdr, pbDeviceLost);
	}

	return E_FAIL;
}
HRESULT XMFCaptureAPIRep::CheckDeviceLost(DEV_BROADCAST_HDR* pHdr, bool* pbDeviceLost)
{
	HRESULT hr = S_OK;
	bool deviceLost = FALSE;
	if (m_CurrentVideoDevice)
	{
		hr = m_CurrentVideoDevice->CheckDeviceLost(pHdr, &deviceLost);
	}

	if (deviceLost == FALSE && SUCCEEDED_Xb(hr))
	{
		if (m_CurrentAudioDevice)
		{
			m_CurrentAudioDevice->CheckDeviceLost(pHdr, &deviceLost);
		}
	}

	if (pbDeviceLost)
	{
		*pbDeviceLost = deviceLost;
	}

	return hr;
}

bool XMFCaptureAPI::IsCapturing()
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->IsCapturing();
	}

	return false;
}
bool XMFCaptureAPIRep::IsCapturing()
{
	return m_IsCapturing;
}

void XMFCaptureAPI::SetOutputPath(XOSString outputPath)
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		m_pRep->SetOutputPath(outputPath);
	}
}
void XMFCaptureAPIRep::SetOutputPath(XOSString outputPath)
{
	m_OutputPath = outputPath;
}

HRESULT XMFCaptureAPI::ReEnumerateDevices()
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->ReEnumerateDevices();
	}

	return E_FAIL;
}
HRESULT XMFCaptureAPIRep::ReEnumerateDevices()
{
	if (m_pXMFCaptureDeviceManager)
	{
		return m_pXMFCaptureDeviceManager->ReEnumerateDevices();
	}

	return E_FAIL;
}

XOSStringList XMFCaptureAPI::GetDevicePairNamesList()
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->GetDevicePairNamesList();
	}

	XOSStringList emptyList;
	return emptyList;
}
XOSStringList XMFCaptureAPIRep::GetDevicePairNamesList()
{
	if (m_pXMFCaptureDeviceManager)
	{
		return m_pXMFCaptureDeviceManager->GetDevicePairNamesList();
	}

	XOSStringList emptyList;
	return emptyList;
}


void XMFCaptureAPI::SetCurrentDevice(XOSString deviceName)
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		m_pRep->SetCurrentDevice(deviceName);
	}
}
void XMFCaptureAPIRep::SetCurrentDevice(XOSString deviceName)
{
	if (!XOSStringsAreTheSame(m_CurrentDeviceName, deviceName))
	{
		m_CurrentDeviceName = deviceName;

		if (m_pXMFCaptureDeviceManager)
		{
			m_pXMFCaptureDeviceManager->GetDeviceByName(deviceName, &m_CurrentVideoDevice, &m_CurrentAudioDevice);
			if (m_CurrentVideoDevice == NULL && m_CurrentAudioDevice == NULL)
			{
				wchar_t  mess[1024];
				swprintf_s(mess, 1024, L"XMFCaptureAPIRep::SetCurrentDevice Failed to get devices with name(%s)\n", deviceName->c_str());
				OutputDebugStringW(mess);
			}
		}
	}
}

XOSString XMFCaptureAPI::GetCurrentDevice()
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->GetCurrentDevice();
	}

	XOSString retVal(new std::wstring(L""));
	return retVal;
}
XOSString XMFCaptureAPIRep::GetCurrentDevice()
{
	XOSString empty(new std::wstring(L""));
	if (m_CurrentDeviceName == empty)
	{
		if (m_pXMFCaptureDeviceManager)
		{
			m_pXMFCaptureDeviceManager->GetDefaultDevices(&m_CurrentVideoDevice, &m_CurrentAudioDevice);
			if (m_CurrentVideoDevice == NULL && m_CurrentAudioDevice == NULL)
			{
				return empty;
			}
			std::pair < DEVICE_PAIR > devPair(m_CurrentVideoDevice, m_CurrentAudioDevice);
			m_CurrentDeviceName = m_pXMFCaptureDeviceManager->GetDevcePairNameWithPreFix(devPair);
		}
	}

	return m_CurrentDeviceName;
}

HRESULT XMFCaptureAPI::StartCapture(bool useOld)
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->StartCapture(useOld);
	}

	return E_FAIL;
}
HRESULT XMFCaptureAPIRep::StartCapture(bool useOld)
{
	HRESULT hr = S_OK;
	m_IsCapturing = true;

	// set devices to capture state
	if (SUCCEEDED_Xb(hr) && m_CurrentVideoDevice)
	{
		hr = m_CurrentVideoDevice->StartCapture();
	}
	if (SUCCEEDED_Xb(hr) && m_CurrentAudioDevice)
	{
		hr = m_CurrentAudioDevice->StartCapture();
	}
	if (SUCCEEDED_Xb(hr))
	{
		m_XMFCaptureEngine = std::make_shared<XMFCaptureEngine>(m_CurrentVideoDevice, m_CurrentAudioDevice, useOld);
		if (m_XMFCaptureEngine)
		{
			hr = m_XMFCaptureEngine->StartRecord(m_OutputPath->c_str());
			//hr = StartPreview(hwnd); // causes error "Some component is already listening to events on this event generator"
			//hr = m_XMFCaptureEngine->StartPreview();
		}
	}
	return hr;
}

HRESULT	XMFCaptureAPI::StartPreview(HWND hwnd)
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->StartPreview(hwnd);
	}

	return E_FAIL;
}
HRESULT XMFCaptureAPIRep::StartPreview(HWND hwnd)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_Xb(hr))
	{
		if (!m_XMFPreview)
		{
			m_XMFPreview = std::make_shared<XMFPreview>();
		}
		
		if (m_XMFPreview)
		{
			hr = m_XMFPreview->StartPreview(hwnd, m_CurrentVideoDevice, m_CurrentAudioDevice);
		}
	}
	if (SUCCEEDED_Xb(hr))
	{
		m_IsPreviewRunning = true;
	}
	return hr;
}

HRESULT	XMFCaptureAPI::StopPreview()
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->StopPreview();
	}

	return E_FAIL;
}
HRESULT XMFCaptureAPIRep::StopPreview()
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_Xb(hr))
	{
		if (m_XMFPreview)
		{
			hr = m_XMFPreview->StopPreview();
		}
	}
	m_IsPreviewRunning = false;
	return hr;
}

HRESULT	XMFCaptureAPI::IsPreviewRunning()
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->IsPreviewRunning();
	}

	return E_FAIL;
}
HRESULT XMFCaptureAPIRep::IsPreviewRunning()
{
	if (m_IsPreviewRunning)
	{
		return S_OK;
	}
	return S_FALSE;
}

HRESULT XMFCaptureAPI::StopCapture()
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->StopCapture();
	}

	return E_FAIL;
}
HRESULT XMFCaptureAPIRep::StopCapture()
{
	HRESULT hr = S_OK;

	if (m_XMFCaptureEngine)
	{
		hr = m_XMFCaptureEngine->StopRecord();
	}

	XOSString empty(new std::wstring(L""));
	m_CurrentDeviceName = empty; // all devices are no longer valid after a shutdown

	m_IsCapturing = false;

	return hr;
}

HRESULT XMFCaptureAPIRep::GetMFDXBufferSource(CComPtr<IUnknown>& pVideoDevice)
{
	CComPtr<IMFSourceResolver> pSourceResolver;
	RETURN_HR_ON_FAIL(MFCreateSourceResolver(&pSourceResolver));

	MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
	CComPtr<IUnknown> pDXBufferSource = NULL;
	RETURN_HR_ON_FAIL(pSourceResolver->CreateObjectFromURL(L"DXBuffer:", MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE, NULL, &objectType, &pDXBufferSource));
	if (pDXBufferSource)
	{
		pVideoDevice = pDXBufferSource;
	}
	return S_OK;
}

HRESULT	XMFCaptureAPI::get_FramesCaptured(unsigned long* pVal)
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->get_FramesCaptured(pVal);
	}
	return E_FAIL;
}
HRESULT	XMFCaptureAPIRep::get_FramesCaptured(unsigned long* pVal)
{
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	if (!m_IsCapturing)
	{
		*pVal = 0;
		return S_OK;
	}
	else
	{
		return m_XMFCaptureEngine->get_FramesCaptured(pVal);
	}
}

HRESULT XMFCaptureAPI::get_FPSForCapture(long* pVal)
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->get_FPSForCapture(pVal);
	}
	return E_FAIL;
}
HRESULT	XMFCaptureAPIRep::get_FPSForCapture(long* pVal)
{
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	if (!m_IsCapturing)
	{
		*pVal = 2997;
		return S_OK;
	}
	else
	{
		return m_XMFCaptureEngine->get_FPSForCapture(pVal);
	}
}

HRESULT XMFCaptureAPI::get_PreviewIsOn(bool* pVal)
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		return m_pRep->get_PreviewIsOn(pVal);
	}
	return E_FAIL;
}
HRESULT	XMFCaptureAPIRep::get_PreviewIsOn(bool* pVal)
{
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	*pVal = m_XMFCaptureEngine->IsPreviewing();
	return S_OK;
}

void XMFCaptureAPI::SetVideoDisplaySize(long width, long height)
{
	LogBeginAPICall(__FUNCTION__);

	if (m_pRep)
	{
		m_pRep->SetVideoDisplaySize(width, height);
	}
}
void XMFCaptureAPIRep::SetVideoDisplaySize(long width, long height)
{
	if (m_XMFPreview)
	{
		m_XMFPreview->SetVideoDisplaySize(width, height);
	}
}