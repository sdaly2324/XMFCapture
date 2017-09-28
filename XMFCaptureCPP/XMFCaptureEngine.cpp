#include "stdafx.h"
#include "XMFCaptureEngine.h"

#include "XMFCaptureEngineWrapper.h"
#include "XMFUtilities.h"
#include "XMFCaptureDevice.h"
#include "XMFAVSourceReader.h"
#include "XMFSinkWriter.h"

#include <Mferror.h>
#include <Mfcaptureengine.h>

class XMFCaptureEngineRep : public IMFCaptureEngineOnEventCallback, public IMFCaptureEngineOnSampleCallback2
{
public:
	XMFCaptureEngineRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld);
	~XMFCaptureEngineRep();

	HRESULT StartPreview(HWND hwnd);
	HRESULT StopPreview();

	HRESULT StartRecord(PCWSTR pszDestinationFile);
	HRESULT StopRecord();

	void XMFCaptureEngineRep::SleepState(bool fSleeping);

	bool IsPreviewing() const;
	bool IsRecording() const;
	HRESULT get_FramesCaptured(unsigned long* pVal) const;
	HRESULT	get_FPSForCapture(long* pVal) const;
	UINT ErrorID() const;

	// IUnknown implementation
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();
	STDMETHODIMP			QueryInterface(REFIID iid, void** ppv);

private:
	XMFCaptureEngineRep(); // never called

	void DestroyCaptureEngine();

	// XMFCaptureEngineOnEventCallback
	STDMETHODIMP OnEvent(IMFMediaEvent* pEvent);

	// IMFCaptureEngineOnSampleCallback2
	STDMETHODIMP OnSample(IMFSample *pSample);
	STDMETHODIMP OnSynchronizedEvent(IMFMediaEvent* pEvent);

	HRESULT OnCaptureEventWhileSleeping(CComPtr<IMFMediaEvent> pEvent);
	void OnCaptureEngineInitialized(HRESULT& hrStatus);
	void OnPreviewStarted(HRESULT& hrStatus);
	void OnRecordStarted(HRESULT& hrStatus);
	void OnPreviewStopped(HRESULT& hrStatus);
	void OnRecordStopped(HRESULT& hrStatus);
	void SetErrorID(HRESULT hr, UINT id);
	volatile long m_nRefCount;                  // COM reference count.

	XMFCaptureEngineWrapper*				m_pCaptureEngineWrapper;

	HANDLE								m_hEvent;

	bool								m_bRecording;
	bool								m_bPreviewing;
	UINT								m_errorID;
	HANDLE								m_hpwrRequest;
	bool								m_fPowerRequestSet;
	bool								m_fSleeping;
	CComAutoCriticalSection				m_criticalSection;
};

XMFCaptureEngine::XMFCaptureEngine(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld)
{
	m_RepPtr = new XMFCaptureEngineRep(pAudioDevice, pVideoDevice, useOld);
}
XMFCaptureEngineRep::XMFCaptureEngineRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld) :
	m_pCaptureEngineWrapper(NULL),
	m_hEvent(NULL),
	m_bRecording(false),
	m_bPreviewing(false),
	m_errorID(0),
	m_hpwrRequest(INVALID_HANDLE_VALUE),
	m_fPowerRequestSet(false),
	m_fSleeping(false),
	m_nRefCount(1)
{
	REASON_CONTEXT  pwrCtxt;
	pwrCtxt.Version = POWER_REQUEST_CONTEXT_VERSION;
	pwrCtxt.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
	pwrCtxt.Reason.SimpleReasonString = L"CaptureEngine is recording!";
	m_hpwrRequest = PowerCreateRequest(&pwrCtxt);
	
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == m_hEvent)
	{
		SUCCEEDED_Xv(HRESULT_FROM_WIN32(GetLastError()));
		return;
	}
	m_pCaptureEngineWrapper = new XMFCaptureEngineWrapper(this, pAudioDevice, pVideoDevice, m_hEvent, useOld);
}

XMFCaptureEngine::~XMFCaptureEngine()
{
	if (m_RepPtr)
	{
		delete m_RepPtr;
		m_RepPtr = NULL;
	}
}
XMFCaptureEngineRep::~XMFCaptureEngineRep()
{
	DestroyCaptureEngine();
}
void XMFCaptureEngineRep::DestroyCaptureEngine()
{
	m_bPreviewing = false;
	m_bRecording = false;
	m_errorID = 0;

	if (NULL != m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
}


HRESULT XMFCaptureEngine::StartRecord(PCWSTR pszDestinationFile)
{
	if (m_RepPtr)
	{
		return m_RepPtr->StartRecord(pszDestinationFile);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::StartRecord(PCWSTR pszDestinationFile)
{
	HRESULT hr = S_OK;

	if (m_bRecording == true)
	{
		return MF_E_INVALIDREQUEST;
	}

	PWSTR pszExt = PathFindExtension(pszDestinationFile);
	if (!(_wcsicmp(pszExt, L".mp4") == 0 || _wcsicmp(pszExt, L".ts") == 0))
	{
		return MF_E_INVALIDMEDIATYPE;
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureEngineWrapper->SetupWriter(pszDestinationFile);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureEngineWrapper->StartRecord();
	}

	m_bRecording = true;
	SUCCEEDED_Xv(hr);
	return hr;
}

HRESULT XMFCaptureEngine::StopRecord()
{
	if (m_RepPtr)
	{
		return m_RepPtr->StopRecord();
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::StopRecord()
{
	HRESULT hr = S_OK;

	if (m_bRecording)
	{
		if (SUCCEEDED_Xb(hr))
		{
			hr = m_pCaptureEngineWrapper->StopRecord();
		}
	}
	return hr;
}



void XMFCaptureEngineRep::OnCaptureEngineInitialized(HRESULT& hrStatus)
{
	if (hrStatus == MF_E_NO_CAPTURE_DEVICES_AVAILABLE)
	{
		hrStatus = S_OK;  // No capture device. Not an application error.
	}
}

void XMFCaptureEngineRep::OnPreviewStarted(HRESULT& hrStatus)
{
	m_bPreviewing = SUCCEEDED_Xb(hrStatus);
}
void XMFCaptureEngineRep::OnPreviewStopped(HRESULT& /*hrStatus*/)
{
	m_bPreviewing = false;
}

void XMFCaptureEngineRep::OnRecordStarted(HRESULT& hrStatus)
{
	m_bRecording = SUCCEEDED_Xb(hrStatus);
}

void XMFCaptureEngineRep::OnRecordStopped(HRESULT& /*hrStatus*/)
{
	m_bRecording = false;
}

void XMFCaptureEngineRep::SetErrorID(HRESULT hr, UINT id)
{
	m_errorID = SUCCEEDED_Xb(hr) ? 0 : id;
}

void XMFCaptureEngine::SleepState(bool fSleeping)
{
	if (m_RepPtr)
	{
		m_RepPtr->SleepState(fSleeping);
	}
}
void XMFCaptureEngineRep::SleepState(bool fSleeping)
{
	m_fSleeping = fSleeping;
}

bool XMFCaptureEngine::IsPreviewing() const
{
	if (m_RepPtr)
	{
		return m_RepPtr->IsPreviewing();
	}
	return false;
}
bool XMFCaptureEngineRep::IsPreviewing() const
{
	return m_bPreviewing;
}

bool XMFCaptureEngine::IsRecording() const
{
	if (m_RepPtr)
	{
		return m_RepPtr->IsRecording();
	}
	return false;
}
bool XMFCaptureEngineRep::IsRecording() const
{
	return m_bRecording;
}

HRESULT XMFCaptureEngine::get_FramesCaptured(unsigned long* pVal) const
{
	if (m_RepPtr)
	{
		return m_RepPtr->get_FramesCaptured(pVal);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::get_FramesCaptured(unsigned long* pVal) const
{
	if (m_pCaptureEngineWrapper)
	{
		return m_pCaptureEngineWrapper->get_FramesCaptured(pVal);
	}
	return E_FAIL;
}

HRESULT	XMFCaptureEngine::get_FPSForCapture(long* pVal) const
{
	if (m_RepPtr)
	{
		return m_RepPtr->get_FPSForCapture(pVal);
	}
	return E_FAIL;
}
HRESULT	XMFCaptureEngineRep::get_FPSForCapture(long* pVal) const
{
	if (m_pCaptureEngineWrapper)
	{
		return m_pCaptureEngineWrapper->get_FPSForCapture(pVal);
	}
	return E_FAIL;
}

UINT XMFCaptureEngine::ErrorID() const
{
	if (m_RepPtr)
	{
		return m_RepPtr->ErrorID();
	}
	return 0;
}
UINT XMFCaptureEngineRep::ErrorID() const
{
	return m_errorID;
}

HRESULT XMFCaptureEngine::StartPreview(HWND hwnd)
{
	if (m_RepPtr)
	{
		return m_RepPtr->StartPreview(hwnd);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::StartPreview(HWND hwnd)
{
	if (m_pCaptureEngineWrapper == NULL)
	{
		return MF_E_NOT_INITIALIZED;
	}

	if (m_bPreviewing == true)
	{
		return S_OK;
	}

	HRESULT hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureEngineWrapper->StartPreview(hwnd);
	}

	if (!m_fPowerRequestSet && m_hpwrRequest != INVALID_HANDLE_VALUE)
	{
		// NOTE:  By calling this, on SOC systems (AOAC enabled), we're asking the system to not go
		// into sleep/connected standby while we're streaming.  However, since we don't want to block
		// the device from ever entering connected standby/sleep, we're going to latch ourselves to
		// the monitor on/off notification (RegisterPowerSettingNotification(GUID_MONITOR_POWER_ON)).
		// On SOC systems, this notification will fire when the user decides to put the device in
		// connected standby mode--we can trap this, turn off our media streams and clear this
		// power set request to allow the device to go into the lower power state.
		m_fPowerRequestSet = (TRUE == PowerSetRequest(m_hpwrRequest, PowerRequestExecutionRequired));
	}
	m_bPreviewing = true;
	return hr;
}

HRESULT XMFCaptureEngine::StopPreview()
{
	if (m_RepPtr)
	{
		return m_RepPtr->StopPreview();
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::StopPreview()
{
	if (m_pCaptureEngineWrapper == NULL)
	{
		return MF_E_NOT_INITIALIZED;
	}

	if (!m_bPreviewing)
	{
		return S_OK;
	}

	HRESULT hr = S_OK;

	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureEngineWrapper->StopPreview();
	}

	if (m_fPowerRequestSet && m_hpwrRequest != INVALID_HANDLE_VALUE)
	{
		PowerClearRequest(m_hpwrRequest, PowerRequestExecutionRequired);
		m_fPowerRequestSet = false;
	}
	return hr;
}

HRESULT XMFCaptureEngineRep::OnCaptureEventWhileSleeping(CComPtr<IMFMediaEvent> pEvent)
{
	HRESULT hr = S_OK;

	// We're about to fall asleep, that means we've just asked the CE to stop the preview
	// and record.  We need to handle it here since our message pump may be gone.
	HRESULT eventStatus = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = pEvent->GetStatus(&eventStatus);
		if (FAILED(eventStatus))
		{
			hr = eventStatus;
		}
	}

	GUID guidType;
	if (SUCCEEDED_Xb(hr))
	{
		hr = pEvent->GetExtendedType(&guidType);
	}

	if (SUCCEEDED_Xb(hr))
	{
		if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STOPPED)
		{
			OnPreviewStopped(eventStatus);
		}
		else if (guidType == MF_CAPTURE_ENGINE_RECORD_STOPPED)
		{
			OnRecordStopped(eventStatus);
		}

		SetEvent(m_hEvent);
	}

	return S_OK;
}
HRESULT XMFCaptureEngineRep::OnSynchronizedEvent(IMFMediaEvent* pEvent)
{
	OutputDebugStringA("Sample format change!\n");

	HRESULT hr = S_OK;

	if (pEvent == NULL)
	{
		return E_INVALIDARG;
	}

	return hr;
}
HRESULT XMFCaptureEngineRep::OnSample(IMFSample *pSample)
{
	HRESULT hr = S_OK;
	if (pSample == NULL)
	{
		return E_INVALIDARG;
	}

	GUID guidValue = GUID_NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = pSample->GetGUID(MF_MT_MAJOR_TYPE, &guidValue);
	}
	if (guidValue == MFMediaType_Audio)
	{
		OutputDebugStringA("XMFCaptureEngineRep::OnSample Audio sample!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	}
	else
	{
		OutputDebugStringA("XMFCaptureEngineRep::OnSample Video sample\n");
		return S_OK;
	}
	return hr;
}

static const bool logMFEvents = false;
void LogMFEvents(std::string eventName)
{
	if (logMFEvents)
	{
		char mess[255];
		sprintf_s(mess, 255, "POOP %s event triggered.\n", eventName.c_str());
		OutputDebugStringA(mess);
	}
}
#define IDS_ERR_INITIALIZE              104
#define IDS_ERR_PREVIEW                 105
#define IDS_ERR_RECORD                  106
#define IDS_ERR_CAPTURE                 107
HRESULT XMFCaptureEngineRep::OnEvent(IMFMediaEvent* pEvent)
{
	// all hell breaks loose if you do not do this (adds a ref)
	CComPtr<IMFMediaEvent> pMyEvent(pEvent);

	HRESULT hr = S_OK;

	if (m_fSleeping)
	{
		return OnCaptureEventWhileSleeping(pMyEvent);
	}
	else
	{
		HRESULT evemtStatus = S_OK;
		if (SUCCEEDED_Xb(hr))
		{
			hr = pMyEvent->GetStatus(&evemtStatus);
		}

		GUID guidType;
		if (SUCCEEDED_Xb(hr))
		{
			if (!SUCCEEDED_Xb(evemtStatus))
			{
				OutputDebugStringA("OnEvent IMFMediaEvent BAD status!\n");
			}
			hr = pMyEvent->GetExtendedType(&guidType);
		}

		if (SUCCEEDED(hr))
		{
			if (guidType == MF_CAPTURE_ENGINE_INITIALIZED)
			{
				LogMFEvents("MF_CAPTURE_ENGINE_INITIALIZED");
				OnCaptureEngineInitialized(evemtStatus);
				SetErrorID(evemtStatus, IDS_ERR_INITIALIZE);
			}
			else if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STARTED)
			{
				LogMFEvents("MF_CAPTURE_ENGINE_PREVIEW_STARTED");
				OnPreviewStarted(evemtStatus);
				SetErrorID(evemtStatus, IDS_ERR_PREVIEW);
			}
			else if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STOPPED)
			{
				LogMFEvents("MF_CAPTURE_ENGINE_PREVIEW_STOPPED");
				OnPreviewStopped(evemtStatus);
				SetErrorID(evemtStatus, IDS_ERR_PREVIEW);
			}
			else if (guidType == MF_CAPTURE_ENGINE_RECORD_STARTED)
			{
				LogMFEvents("MF_CAPTURE_ENGINE_RECORD_STARTED");
				OnRecordStarted(evemtStatus);
				SetErrorID(evemtStatus, IDS_ERR_RECORD);
			}
			else if (guidType == MF_CAPTURE_ENGINE_RECORD_STOPPED)
			{
				LogMFEvents("MF_CAPTURE_ENGINE_RECORD_STOPPED");
				OnRecordStopped(evemtStatus);
				SetErrorID(evemtStatus, IDS_ERR_RECORD);
			}
			else if (guidType == MF_CAPTURE_ENGINE_ERROR)
			{
				LogMFEvents("MF_CAPTURE_ENGINE_ERROR");
				DestroyCaptureEngine();
				SetErrorID(evemtStatus, IDS_ERR_CAPTURE);
			}
			else if (FAILED(evemtStatus))
			{
				SetErrorID(evemtStatus, IDS_ERR_CAPTURE);
			}
			else
			{
				OLECHAR* guidString;
				StringFromCLSID(guidType, &guidString);
				std::wstring ws(guidString);
				std::string str(ws.begin(), ws.end());
				LogMFEvents(str);
			}
		}

		SetEvent(m_hEvent);
		if (FAILED(evemtStatus))
		{
			return evemtStatus;
		}
	}

	return hr;
}

ULONG XMFCaptureEngineRep::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}

ULONG XMFCaptureEngineRep::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}

STDMETHODIMP XMFCaptureEngineRep::QueryInterface(REFIID iid, void** ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}
	if (iid == __uuidof(IUnknown))
	{
		*ppv = static_cast<IUnknown*>(static_cast<IMFCaptureEngineOnEventCallback*>(this));
	}
	else if (iid == __uuidof(IMFCaptureEngineOnEventCallback))
	{
		*ppv = static_cast<IMFCaptureEngineOnEventCallback*>(this);
	}
	else if (iid == __uuidof(IMFCaptureEngineOnSampleCallback2))
	{
		*ppv = static_cast<IMFCaptureEngineOnSampleCallback2*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}