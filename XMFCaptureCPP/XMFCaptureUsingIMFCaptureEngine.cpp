#include "stdafx.h"
#include "XMFCaptureUsingIMFCaptureEngine.h"

#include <mfcaptureengine.h>
#include <Mferror.h>

class XMFCaptureUsingIMFCaptureEngineRep : public IXMFCaptureEngine, public IMFCaptureEngineOnEventCallback
{
public:
	XMFCaptureUsingIMFCaptureEngineRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice);
	~XMFCaptureUsingIMFCaptureEngineRep();

	HRESULT SetupWriter(PCWSTR pszDestinationFile);
	CComPtr<IMFMediaType> GetAudioMTypeFromSource();
	CComPtr<IMFMediaType> GetVideoMTypeFromSource();
	HRESULT AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType);
	HRESULT AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType);

	HRESULT StartRecord();
	HRESULT StopRecord();

	HRESULT StartPreview(HWND hwnd);
	HRESULT StopPreview();

	virtual bool IsPreviewing() const;
	virtual bool IsRecording() const;
	HRESULT GetStatistics(MF_SINK_WRITER_STATISTICS *pStats);

private:
	// XMFCaptureEngineOnEventCallback
	HANDLE m_hEvent = INVALID_HANDLE_VALUE;
	STDMETHODIMP OnEvent(IMFMediaEvent* pEvent);

	bool m_bRecording = false;
	bool m_bPreviewing = false;

	void OnCaptureEngineInitialized(HRESULT& hrStatus);
	void OnPreviewStarted(HRESULT& hrStatus);
	void OnRecordStarted(HRESULT& hrStatus);
	void OnPreviewStopped(HRESULT& hrStatus);
	void OnRecordStopped(HRESULT& hrStatus);
	void DestroyCaptureEngine();

	// IUnknown implementation
	volatile long m_nRefCount = 1; // COM reference count.
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();
	STDMETHODIMP			QueryInterface(REFIID iid, void** ppv);

	CComPtr<IMFCaptureSink> GetPreviewSink();
	CComPtr<IMFCaptureSink> GetCaptureSink();
	CComPtr<IMFCaptureSource> GetCaptureSource();

	CComPtr<IMFCaptureEngine>			m_pIMFCaptureEngine = NULL;
	CComPtr<IMFSinkWriter>				m_pIMFSinkWriter = NULL;
	CComPtr<IMFCapturePreviewSink>		m_pIMFCapturePreviewSink = NULL;
	CComPtr<IMFCaptureRecordSink>		m_pIMFCaptureRecordSink = NULL;
	DWORD								m_dwVideoSinkStreamIndex = 0;
	DWORD								m_dwAudioSinkStreamIndex = 0;
};

XMFCaptureUsingIMFCaptureEngine::XMFCaptureUsingIMFCaptureEngine(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice)
{
	m_pRep = new XMFCaptureUsingIMFCaptureEngineRep(pAudioDevice, pVideoDevice);
}
XMFCaptureUsingIMFCaptureEngineRep::XMFCaptureUsingIMFCaptureEngineRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice)
{
	HRESULT hr = S_OK;
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hEvent == INVALID_HANDLE_VALUE)
	{
		SUCCEEDED_Xv(HRESULT_FROM_WIN32(GetLastError()));
		return;
	}
	CComPtr<IMFCaptureEngineClassFactory> pFactory = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = CoCreateInstance(CLSID_MFCaptureEngineClassFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pFactory->CreateInstance(CLSID_MFCaptureEngine, IID_PPV_ARGS(&m_pIMFCaptureEngine));
	}
	if (SUCCEEDED_Xb(hr))
	{
		CComPtr<IUnknown> pAudioActivate = NULL;
		if (SUCCEEDED_Xb(hr) && pAudioDevice)
		{
			hr = pAudioDevice->GetIMFActivate(pAudioActivate);
		}
		CComPtr<IUnknown> pVideoActivate = NULL;
		if (SUCCEEDED_Xb(hr) && pVideoDevice)
		{
			hr = pVideoDevice->GetIMFActivate(pVideoActivate);
		}
		CComPtr<IMFAttributes> pAttributes = NULL;
		hr = m_pIMFCaptureEngine->Initialize(this, pAttributes, pAudioActivate, pVideoActivate);
		WaitForSingleObject(m_hEvent, INFINITE);
	}
}
XMFCaptureUsingIMFCaptureEngine::~XMFCaptureUsingIMFCaptureEngine()
{
	if (m_pRep)
	{
		delete m_pRep;
	}
}
XMFCaptureUsingIMFCaptureEngineRep::~XMFCaptureUsingIMFCaptureEngineRep()
{
	DestroyCaptureEngine();
}
void XMFCaptureUsingIMFCaptureEngineRep::DestroyCaptureEngine()
{
	m_bPreviewing = false;
	m_bRecording = false;
	CloseHandle(m_hEvent);
}
CComPtr<IMFCaptureSink> XMFCaptureUsingIMFCaptureEngineRep::GetPreviewSink()
{
	HRESULT hr = S_OK;
	CComPtr<IMFCaptureSink>	pPreviewSink = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIMFCaptureEngine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_PREVIEW, &pPreviewSink);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return pPreviewSink;
	}
	return NULL;
}
CComPtr<IMFCaptureSink> XMFCaptureUsingIMFCaptureEngineRep::GetCaptureSink()
{
	HRESULT hr = S_OK;
	CComPtr<IMFCaptureSink>	pCaptureSink = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIMFCaptureEngine->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_RECORD, &pCaptureSink);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return pCaptureSink;
	}
	return NULL;
}
CComPtr<IMFCaptureSource> XMFCaptureUsingIMFCaptureEngineRep::GetCaptureSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFCaptureSource> pCaptureSource = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIMFCaptureEngine->GetSource(&pCaptureSource);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return pCaptureSource;
	}
	return NULL;
}
HRESULT XMFCaptureUsingIMFCaptureEngine::SetupWriter(PCWSTR pszDestinationFile)
{
	if (m_pRep)
	{
		return m_pRep->SetupWriter(pszDestinationFile);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngineRep::SetupWriter(PCWSTR pszDestinationFile)
{
	HRESULT hr = S_OK;

	CComPtr<IMFCaptureSource> pIMFCaptureSource = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = GetCaptureSink()->QueryInterface(IID_PPV_ARGS(&m_pIMFCaptureRecordSink));
	}
	if (SUCCEEDED_Xb(hr))
	{
		pIMFCaptureSource = GetCaptureSource();
		if (pIMFCaptureSource == NULL)
		{
			hr = E_FAIL;
		}
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIMFCaptureRecordSink->RemoveAllStreams();
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIMFCaptureRecordSink->SetOutputFileName(pszDestinationFile);
	}
	return hr;
}
CComPtr<IMFMediaType> XMFCaptureUsingIMFCaptureEngine::GetAudioMTypeFromSource()
{
	if (m_pRep)
	{
		return m_pRep->GetAudioMTypeFromSource();
	}
	return NULL;
}
CComPtr<IMFMediaType> XMFCaptureUsingIMFCaptureEngineRep::GetAudioMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	CComPtr<IMFCaptureSource> pSource = GetCaptureSource();
	if (pSource)
	{
		hr = pSource->GetAvailableDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO, 0, &retVal);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return retVal;
	}
	return NULL;
}
CComPtr<IMFMediaType> XMFCaptureUsingIMFCaptureEngine::GetVideoMTypeFromSource()
{
	if (m_pRep)
	{
		return m_pRep->GetVideoMTypeFromSource();
	}
	return NULL;
}
CComPtr<IMFMediaType> XMFCaptureUsingIMFCaptureEngineRep::GetVideoMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	CComPtr<IMFCaptureSource> pSource = GetCaptureSource();
	// TODO
	// DETECT INPUT FORMAT! IS IT EVEN POSSIBLE!
	//DWORD formatWeWant = 0;	// WEBCAM			640		30		YUY2
	//DWORD formatWeWant = 1;	// XI100DUSB-SDI	1080p	5994	YUY2
	DWORD formatWeWant = 64;	// XI100DUSB-SDI	720p	5994	YUY2
	hr = pSource->GetAvailableDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, formatWeWant, &retVal);
	if (SUCCEEDED_Xb(hr))
	{
		return retVal;
	}
	return NULL;
}
HRESULT XMFCaptureUsingIMFCaptureEngine::AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType)
{
	if (m_pRep)
	{
		return m_pRep->AddVideoStream(pVideoOutputMediaType);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngineRep::AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType)
{
	HRESULT hr = S_OK;
	if (m_pIMFCaptureRecordSink == NULL)
	{
		return E_FAIL;
	}
	if (m_pIMFCaptureRecordSink)
	{
		hr = m_pIMFCaptureRecordSink->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, pVideoOutputMediaType, NULL, &m_dwVideoSinkStreamIndex);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return hr;
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngine::AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType)
{
	if (m_pRep)
	{
		return m_pRep->AddAudioStream(pAudioOutputMediaType);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngineRep::AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType)
{
	HRESULT hr = S_OK;
	if (m_pIMFCaptureRecordSink == NULL)
	{
		return E_FAIL;
	}
	if (m_pIMFCaptureRecordSink)
	{
		hr = m_pIMFCaptureRecordSink->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO, pAudioOutputMediaType, NULL, &m_dwAudioSinkStreamIndex);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return m_dwVideoSinkStreamIndex;
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngine::StartRecord()
{
	if (m_pRep)
	{
		return m_pRep->StartRecord();
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngineRep::StartRecord()
{
	HRESULT hr = S_OK;
	if (m_bRecording == true)
	{
		return MF_E_INVALIDREQUEST;
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIMFCaptureEngine->StartRecord();
	}
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEvent, INFINITE);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = GetCaptureSink()->GetService(m_dwVideoSinkStreamIndex, GUID_NULL, IID_IMFSinkWriter, (IUnknown**)&m_pIMFSinkWriter);
	}
	if (SUCCEEDED_Xb(hr))
	{
		m_bRecording = true;
	}
	return hr;
}
HRESULT XMFCaptureUsingIMFCaptureEngine::StopRecord()
{
	if (m_pRep)
	{
		return m_pRep->StopRecord();
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngineRep::StopRecord()
{
	HRESULT hr = S_OK;
	hr = m_pIMFCaptureEngine->StopRecord(TRUE, TRUE);
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEvent, INFINITE);
	}
	return hr;
}
HRESULT XMFCaptureUsingIMFCaptureEngine::StartPreview(HWND hwnd)
{
	if (m_pRep)
	{
		return m_pRep->StartPreview(hwnd);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngineRep::StartPreview(HWND hwnd)
{
	HRESULT hr = S_OK;
	if (m_pIMFCapturePreviewSink == NULL)
	{
		CComPtr<IMFCaptureSink> pPreviewSink = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			pPreviewSink = GetPreviewSink();
			if (pPreviewSink == NULL)
			{
				return E_FAIL;
			}
		}
		if (SUCCEEDED_Xb(hr))
		{
			hr = pPreviewSink->QueryInterface(IID_PPV_ARGS(&m_pIMFCapturePreviewSink));
		}
		CComPtr<IMFCaptureSource> pPreviewSource = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			pPreviewSource = GetCaptureSource();
			if (pPreviewSource == NULL)
			{
				return E_FAIL;
			}
		}
		DWORD streamCount = 0;
		if (SUCCEEDED_Xb(hr) && pPreviewSource)
		{
			hr = pPreviewSource->GetDeviceStreamCount(&streamCount);
		}
		for (DWORD i = 0; i < streamCount; i++)
		{
			CComPtr<IMFMediaType> pMediaType = NULL;
			if (SUCCEEDED_Xb(hr))
			{
				hr = pPreviewSource->GetCurrentDeviceMediaType(i, &pMediaType);
			}
			GUID guidValue = GUID_NULL;
			if (SUCCEEDED_Xb(hr))
			{
				hr = pMediaType->GetGUID(MF_MT_MAJOR_TYPE, &guidValue);
			}
			if (SUCCEEDED_Xb(hr))
			{
				DWORD dwSinkStreamIndex = 0;
				if (guidValue == MFMediaType_Video)
				{
					hr = m_pIMFCapturePreviewSink->SetRenderHandle(hwnd);
					hr = m_pIMFCapturePreviewSink->AddStream(i, pMediaType, NULL, &dwSinkStreamIndex);
				}
				else if (guidValue == MFMediaType_Audio)
				{
					//hr = m_pPreview->AddStream(i, pMediaType, NULL, &dwSinkStreamIndex); // not only does this give you no hr error and no audio but it breaks the video preview stream with "The request is invalid in the current state" in the OnEvent callback
				}
			}
		}
	}
	hr = m_pIMFCaptureEngine->StartPreview();
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEvent, INFINITE);
	}
	return hr;
}
HRESULT XMFCaptureUsingIMFCaptureEngine::StopPreview()
{
	if (m_pRep)
	{
		return m_pRep->StopPreview();
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngineRep::StopPreview()
{
	HRESULT hr = m_pIMFCaptureEngine->StopPreview();
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEvent, INFINITE);
	}
	return hr;
}
bool XMFCaptureUsingIMFCaptureEngine::IsPreviewing() const
{
	if (m_pRep)
	{
		return m_pRep->IsPreviewing();
	}
	return false;
}
bool XMFCaptureUsingIMFCaptureEngineRep::IsPreviewing() const
{
	return m_bPreviewing;
}
bool XMFCaptureUsingIMFCaptureEngine::IsRecording() const
{
	if (m_pRep)
	{
		return m_pRep->IsRecording();
	}
	return false;
}
bool XMFCaptureUsingIMFCaptureEngineRep::IsRecording() const
{
	return m_bRecording;
}
HRESULT XMFCaptureUsingIMFCaptureEngine::GetStatistics(MF_SINK_WRITER_STATISTICS* pStats)
{
	if (m_pRep)
	{
		return m_pRep->GetStatistics(pStats);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFCaptureEngineRep::GetStatistics(MF_SINK_WRITER_STATISTICS* pStats)
{
	HRESULT hr = S_OK;
	if (pStats == NULL)
	{
		return E_INVALIDARG;
	}
	if (SUCCEEDED_Xb(hr) && m_pIMFSinkWriter)
	{
		hr = m_pIMFSinkWriter->GetStatistics(m_dwVideoSinkStreamIndex, pStats);
	}
	return hr;
}
static const bool logMFEvents = false;
void LogMFEvents(std::string eventName)
{
	if (logMFEvents)
	{
		char mess[255];
		sprintf_s(mess, 255, "%s event triggered.\n", eventName.c_str());
		OutputDebugStringA(mess);
	}
}
HRESULT XMFCaptureUsingIMFCaptureEngineRep::OnEvent(IMFMediaEvent* pEvent)
{
	// all hell breaks loose if you do not do this (adds a ref)
	CComPtr<IMFMediaEvent> pMyEvent(pEvent);
	HRESULT hr = S_OK;
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
		}
		else if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STARTED)
		{
			LogMFEvents("MF_CAPTURE_ENGINE_PREVIEW_STARTED");
			OnPreviewStarted(evemtStatus);
		}
		else if (guidType == MF_CAPTURE_ENGINE_PREVIEW_STOPPED)
		{
			LogMFEvents("MF_CAPTURE_ENGINE_PREVIEW_STOPPED");
			OnPreviewStopped(evemtStatus);
		}
		else if (guidType == MF_CAPTURE_ENGINE_RECORD_STARTED)
		{
			LogMFEvents("MF_CAPTURE_ENGINE_RECORD_STARTED");
			OnRecordStarted(evemtStatus);
		}
		else if (guidType == MF_CAPTURE_ENGINE_RECORD_STOPPED)
		{
			LogMFEvents("MF_CAPTURE_ENGINE_RECORD_STOPPED");
			OnRecordStopped(evemtStatus);
		}
		else if (guidType == MF_CAPTURE_ENGINE_ERROR)
		{
			LogMFEvents("MF_CAPTURE_ENGINE_ERROR");
			DestroyCaptureEngine();
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
	return hr;
}
void XMFCaptureUsingIMFCaptureEngineRep::OnCaptureEngineInitialized(HRESULT& hrStatus)
{
	if (hrStatus == MF_E_NO_CAPTURE_DEVICES_AVAILABLE)
	{
		hrStatus = S_OK;  // No capture device. Not an application error.
	}
}
void XMFCaptureUsingIMFCaptureEngineRep::OnPreviewStarted(HRESULT& hrStatus)
{
	m_bPreviewing = SUCCEEDED_Xb(hrStatus);
}
void XMFCaptureUsingIMFCaptureEngineRep::OnPreviewStopped(HRESULT& /*hrStatus*/)
{
	m_bPreviewing = false;
}

void XMFCaptureUsingIMFCaptureEngineRep::OnRecordStarted(HRESULT& hrStatus)
{
	m_bRecording = SUCCEEDED_Xb(hrStatus);
}

void XMFCaptureUsingIMFCaptureEngineRep::OnRecordStopped(HRESULT& /*hrStatus*/)
{
	m_bRecording = false;
}
ULONG XMFCaptureUsingIMFCaptureEngineRep::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}
ULONG XMFCaptureUsingIMFCaptureEngineRep::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}
STDMETHODIMP XMFCaptureUsingIMFCaptureEngineRep::QueryInterface(REFIID iid, void** ppv)
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
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

