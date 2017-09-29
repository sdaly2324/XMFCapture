#include "stdafx.h"
#include "XMFCaptureEngineWrapper.h"

#include "XMFCaptureDevice.h"
#include "XMFAVSourceReader.h"
#include "XMFSinkWriter.h"

#include <Mfcaptureengine.h>
#include <Mferror.h>

// old
#include "XMFSinkWriterCallback.h"	
#include "XMFSourceReaderCallback.h"

class IXMFCaptureEngine
{
public:
	virtual HRESULT SetupWriter(PCWSTR pszDestinationFile) = 0;
	virtual CComPtr<IMFMediaType> GetAudioMTypeFromSource() = 0;
	virtual CComPtr<IMFMediaType> GetVideoMTypeFromSource() = 0;
	virtual HRESULT AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType) = 0;
	virtual HRESULT AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType) = 0;

	virtual HRESULT StartRecord() = 0;
	virtual HRESULT StopRecord() = 0;

	virtual HRESULT StartPreview(HWND hwnd) = 0;
	virtual HRESULT StopPreview() = 0;

	virtual bool IsPreviewing() const = 0;
	virtual bool IsRecording() const = 0;
	virtual HRESULT GetStatistics(MF_SINK_WRITER_STATISTICS *pStats) = 0;
};

class XMFCaptureEngineWrapperRepOLD : public IXMFCaptureEngine,  public IMFCaptureEngineOnSampleCallback2
{
public:
	XMFCaptureEngineWrapperRepOLD(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice);
	~XMFCaptureEngineWrapperRepOLD();

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

	// IUnknown implementation
	volatile long m_nRefCount = 1; // COM reference count.
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();
	STDMETHODIMP			QueryInterface(REFIID iid, void** ppv);

private:
	CComPtr<IMFMediaSource> GetAggregateMediaSourceOLD(CComPtr<IMFMediaSource> pAudioSource, CComPtr<IMFMediaSource> pVideoSource);

	// IMFCaptureEngineOnSampleCallback2
	STDMETHODIMP OnSample(IMFSample *pSample);
	STDMETHODIMP OnSynchronizedEvent(IMFMediaEvent* pEvent);

	CComPtr<IMFMediaSource>	m_pAggregatSourceOLD = NULL;
	XMFAVSourceReader* m_pXMFAVSourceReaderOLD = NULL;
	XMFSinkWriter* m_pXMFSinkWriterOLD = NULL;
	bool m_bRecording = false;
	const UINT IDS_ERR_INITIALIZE	= 104;
	const UINT IDS_ERR_PREVIEW		= 105;
	const UINT IDS_ERR_RECORD		= 106;
	const UINT IDS_ERR_CAPTURE		= 107;
	DWORD m_dwVideoSinkStreamIndex = 0;
};
XMFCaptureEngineWrapperRepOLD::XMFCaptureEngineWrapperRepOLD(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice)
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaSource> pAudioSource = NULL;
	if (SUCCEEDED_Xb(hr) && pAudioDevice)
	{
		hr = pAudioDevice->GetIMFMediaSource(pAudioSource);
		m_pAggregatSourceOLD = pAudioSource;
	}
	CComPtr<IMFMediaSource> pVideoSource = NULL;
	if (SUCCEEDED_Xb(hr) && pVideoDevice)
	{
		hr = pVideoDevice->GetIMFMediaSource(pVideoSource);
		m_pAggregatSourceOLD = pVideoSource;
	}
	if (SUCCEEDED_Xb(hr) && pAudioSource && pVideoSource)
	{
		m_pAggregatSourceOLD = GetAggregateMediaSourceOLD(pAudioSource, pVideoSource);
	}
}
XMFCaptureEngineWrapperRepOLD::~XMFCaptureEngineWrapperRepOLD()
{

}

HRESULT XMFCaptureEngineWrapperRepOLD::SetupWriter(PCWSTR pszDestinationFile)
{
	if (m_pXMFSinkWriterOLD)
	{
		delete m_pXMFSinkWriterOLD;
	}
	m_pXMFSinkWriterOLD = new XMFSinkWriter(pszDestinationFile);
	if (m_pXMFSinkWriterOLD)
	{
		if (m_pXMFAVSourceReaderOLD)
		{
			delete m_pXMFAVSourceReaderOLD;
		}
		m_pXMFAVSourceReaderOLD = new XMFAVSourceReader(m_pXMFSinkWriterOLD, m_pAggregatSourceOLD);
	}
	return S_OK;
}
CComPtr<IMFMediaType> XMFCaptureEngineWrapperRepOLD::GetAudioMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	if (m_pXMFAVSourceReaderOLD)
	{
		hr = m_pXMFAVSourceReaderOLD->GetAudioInputMediaType(retVal);
		if (SUCCEEDED_Xb(hr))
		{
			return retVal;
		}
	}
	return NULL;
}
CComPtr<IMFMediaType> XMFCaptureEngineWrapperRepOLD::GetVideoMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	if (m_pXMFAVSourceReaderOLD)
	{
		hr = m_pXMFAVSourceReaderOLD->GetVideoInputMediaType(retVal);
		if (SUCCEEDED_Xb(hr))
		{
			return retVal;
		}
	}
	return NULL;
}
HRESULT XMFCaptureEngineWrapperRepOLD::AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType)
{
	if (m_pXMFSinkWriterOLD == NULL)
	{
		return E_FAIL;
	}
	HRESULT  hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pXMFSinkWriterOLD->AddStream(pVideoOutputMediaType, &m_dwVideoSinkStreamIndex);
	}
	CComPtr<IMFMediaType> pInputMediaType = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		m_pXMFAVSourceReaderOLD->GetVideoInputMediaType(pInputMediaType);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pXMFSinkWriterOLD->SetInputMediaType(m_dwVideoSinkStreamIndex, pInputMediaType, NULL);
	}
	return hr;
}
HRESULT XMFCaptureEngineWrapperRepOLD::AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType)
{
	if (m_pXMFSinkWriterOLD == NULL)
	{
		return E_FAIL;
	}
	HRESULT  hr = S_OK;
	DWORD aideoSinkStreamIndex = 0;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pXMFSinkWriterOLD->AddStream(pAudioOutputMediaType, &aideoSinkStreamIndex);
	}
	CComPtr<IMFMediaType> pInputMediaType = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		m_pXMFAVSourceReaderOLD->GetAudioInputMediaType(pInputMediaType);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pXMFSinkWriterOLD->SetInputMediaType(aideoSinkStreamIndex, pInputMediaType, NULL);
	}
	return hr;
}
HRESULT XMFCaptureEngineWrapperRepOLD::OnSynchronizedEvent(IMFMediaEvent* pEvent)
{
	OutputDebugStringA("Sample format change!\n");

	HRESULT hr = S_OK;

	if (pEvent == NULL)
	{
		return E_INVALIDARG;
	}

	return hr;
}
HRESULT XMFCaptureEngineWrapperRepOLD::OnSample(IMFSample *pSample)
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
HRESULT XMFCaptureEngineWrapperRepOLD::StartRecord()
{
	HRESULT hr = S_OK;
	if (m_bRecording == true)
	{
		return MF_E_INVALIDREQUEST;
	}
	if (m_pXMFSinkWriterOLD)
	{
		hr = m_pXMFSinkWriterOLD->BeginWriting();
	}
	if (SUCCEEDED_Xb(hr) && m_pXMFAVSourceReaderOLD)
	{
		hr = m_pXMFAVSourceReaderOLD->Start();
	}
	else
	{
		hr = E_FAIL;
	}
	if (SUCCEEDED_Xb(hr))
	{
		m_bRecording = true;
	}
	return hr;
}
HRESULT XMFCaptureEngineWrapperRepOLD::StopRecord()
{
	HRESULT hr = S_OK;
	if (m_pXMFSinkWriterOLD)
	{
		hr = m_pXMFSinkWriterOLD->EndWriting();
	}
	delete m_pXMFAVSourceReaderOLD;
	m_pXMFAVSourceReaderOLD = NULL;
	return hr;
}
HRESULT XMFCaptureEngineWrapperRepOLD::StartPreview(HWND /*hwnd*/)
{
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRepOLD::StopPreview()
{
	return E_FAIL;
}
bool XMFCaptureEngineWrapperRepOLD::IsPreviewing() const
{
	return false;
}
bool XMFCaptureEngineWrapperRepOLD::IsRecording() const
{
	return m_bRecording;
}
HRESULT XMFCaptureEngineWrapperRepOLD::GetStatistics(MF_SINK_WRITER_STATISTICS *pStats)
{
	HRESULT hr = S_OK;
	if (pStats == NULL)
	{
		return E_INVALIDARG;
	}
	if (SUCCEEDED_Xb(hr) && m_pXMFSinkWriterOLD)
	{
		m_pXMFSinkWriterOLD->GetStatistics(m_dwVideoSinkStreamIndex, pStats);
	}
	return hr;
}
CComPtr<IMFMediaSource> XMFCaptureEngineWrapperRepOLD::GetAggregateMediaSourceOLD(CComPtr<IMFMediaSource> pAudioSource, CComPtr<IMFMediaSource> pVideoSource)
{
	CComPtr<IMFCollection> pCollection = NULL;
	HRESULT hr = MFCreateCollection(&pCollection);
	if (SUCCEEDED_Xb(hr))
	{
		hr = pCollection->AddElement(pAudioSource);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pCollection->AddElement(pVideoSource);
	}
	CComPtr<IMFMediaSource> pVASource = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateAggregateSource(pCollection, &pVASource);
	}
	if (SUCCEEDED_Xb(hr) && pVASource)
	{
		return pVASource;
	}
	return NULL;
}
ULONG XMFCaptureEngineWrapperRepOLD::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}
ULONG XMFCaptureEngineWrapperRepOLD::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}
STDMETHODIMP XMFCaptureEngineWrapperRepOLD::QueryInterface(REFIID iid, void** ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}
	if (iid == __uuidof(IUnknown))
	{
		*ppv = static_cast<IUnknown*>(static_cast<IMFCaptureEngineOnSampleCallback2*>(this));
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




class XMFCaptureEngineWrapperRepNEW : public IXMFCaptureEngine, public IMFCaptureEngineOnEventCallback
{
public:
	XMFCaptureEngineWrapperRepNEW(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice);
	~XMFCaptureEngineWrapperRepNEW();

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

	CComPtr<IMFCaptureSink> GetPreviewSinkNEW();
	CComPtr<IMFCaptureSink> GetCaptureSinkNEW();
	CComPtr<IMFCaptureSource> GetCaptureSourceNEW();
	
	CComPtr<IMFCaptureEngine>			m_pEngineNEW = NULL;
	CComPtr<IMFSinkWriter>				m_pVideoSinkWriterNEW = NULL;
	CComPtr<IMFCapturePreviewSink>		m_pPreviewNEW = NULL;
	CComPtr<IMFCaptureRecordSink>		m_pCaptureRecordSinkNEW = NULL;
	DWORD								m_dwVideoSinkStreamIndex = 0;
	DWORD								m_dwAudioSinkStreamIndex = 0;
};
XMFCaptureEngineWrapperRepNEW::XMFCaptureEngineWrapperRepNEW(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice)
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
		hr = pFactory->CreateInstance(CLSID_MFCaptureEngine, IID_PPV_ARGS(&m_pEngineNEW));
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
		hr = m_pEngineNEW->Initialize(this, pAttributes, pAudioActivate, pVideoActivate);
		WaitForSingleObject(m_hEvent, INFINITE);
	}
}
XMFCaptureEngineWrapperRepNEW::~XMFCaptureEngineWrapperRepNEW()
{
	DestroyCaptureEngine();
}
void XMFCaptureEngineWrapperRepNEW::DestroyCaptureEngine()
{
	m_bPreviewing = false;
	m_bRecording = false;
	CloseHandle(m_hEvent);
}
CComPtr<IMFCaptureSink> XMFCaptureEngineWrapperRepNEW::GetPreviewSinkNEW()
{
	HRESULT hr = S_OK;
	CComPtr<IMFCaptureSink>	pPreviewSink = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pEngineNEW->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_PREVIEW, &pPreviewSink);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return pPreviewSink;
	}
	return NULL;
}
CComPtr<IMFCaptureSink> XMFCaptureEngineWrapperRepNEW::GetCaptureSinkNEW()
{
	HRESULT hr = S_OK;
	CComPtr<IMFCaptureSink>	pCaptureSink = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pEngineNEW->GetSink(MF_CAPTURE_ENGINE_SINK_TYPE_RECORD, &pCaptureSink);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return pCaptureSink;
	}
	return NULL;
}
CComPtr<IMFCaptureSource> XMFCaptureEngineWrapperRepNEW::GetCaptureSourceNEW()
{
	HRESULT hr = S_OK;
	CComPtr<IMFCaptureSource> pCaptureSource = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pEngineNEW->GetSource(&pCaptureSource);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return pCaptureSource;
	}
	return NULL;
}
HRESULT XMFCaptureEngineWrapperRepNEW::SetupWriter(PCWSTR pszDestinationFile)
{
	HRESULT hr = S_OK;
	
	CComPtr<IMFCaptureSource> pCaptureSourceNEW = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = GetCaptureSinkNEW()->QueryInterface(IID_PPV_ARGS(&m_pCaptureRecordSinkNEW));
	}
	if (SUCCEEDED_Xb(hr))
	{
		pCaptureSourceNEW = GetCaptureSourceNEW();
		if (pCaptureSourceNEW == NULL)
		{
			hr = E_FAIL;
		}
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureRecordSinkNEW->RemoveAllStreams();
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureRecordSinkNEW->SetOutputFileName(pszDestinationFile);
	}
	return hr;
}
CComPtr<IMFMediaType> XMFCaptureEngineWrapperRepNEW::GetAudioMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	CComPtr<IMFCaptureSource> pSource = GetCaptureSourceNEW();
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
CComPtr<IMFMediaType> XMFCaptureEngineWrapperRepNEW::GetVideoMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	CComPtr<IMFCaptureSource> pSource = GetCaptureSourceNEW();
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
HRESULT XMFCaptureEngineWrapperRepNEW::AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType)
{
	HRESULT hr = S_OK;
	if (m_pCaptureRecordSinkNEW == NULL)
	{
		return E_FAIL;
	}
	if (m_pCaptureRecordSinkNEW)
	{
		hr =  m_pCaptureRecordSinkNEW->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, pVideoOutputMediaType, NULL, &m_dwVideoSinkStreamIndex);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return hr;
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRepNEW::AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType)
{
	HRESULT hr = S_OK;
	if (m_pCaptureRecordSinkNEW == NULL)
	{
		return E_FAIL;
	}
	if (m_pCaptureRecordSinkNEW)
	{
		hr = m_pCaptureRecordSinkNEW->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO, pAudioOutputMediaType, NULL, &m_dwAudioSinkStreamIndex);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return m_dwVideoSinkStreamIndex;
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRepNEW::StartRecord()
{
	HRESULT hr = S_OK;
	if (m_bRecording == true)
	{
		return MF_E_INVALIDREQUEST;
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pEngineNEW->StartRecord();
	}
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEvent, INFINITE);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = GetCaptureSinkNEW()->GetService(m_dwVideoSinkStreamIndex, GUID_NULL, IID_IMFSinkWriter, (IUnknown**)&m_pVideoSinkWriterNEW);
	}
	if (SUCCEEDED_Xb(hr))
	{
		m_bRecording = true;
	}
	return hr;
}
HRESULT XMFCaptureEngineWrapperRepNEW::StopRecord()
{
	HRESULT hr = S_OK;
	hr = m_pEngineNEW->StopRecord(TRUE, TRUE);
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEvent, INFINITE);
	}
	return hr;
}
HRESULT XMFCaptureEngineWrapperRepNEW::StartPreview(HWND hwnd)
{
	HRESULT hr = S_OK;
	if (m_pPreviewNEW == NULL)
	{
		CComPtr<IMFCaptureSink> pPreviewSink = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			pPreviewSink = GetPreviewSinkNEW();
			if (pPreviewSink == NULL)
			{
				return E_FAIL;
			}
		}
		if (SUCCEEDED_Xb(hr))
		{
			hr = pPreviewSink->QueryInterface(IID_PPV_ARGS(&m_pPreviewNEW));
		}
		CComPtr<IMFCaptureSource> pPreviewSource = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			pPreviewSource = GetCaptureSourceNEW();
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
					hr = m_pPreviewNEW->SetRenderHandle(hwnd);
					hr = m_pPreviewNEW->AddStream(i, pMediaType, NULL, &dwSinkStreamIndex);
				}
				else if (guidValue == MFMediaType_Audio)
				{
					//hr = m_pPreview->AddStream(i, pMediaType, NULL, &dwSinkStreamIndex); // not only does this give you no hr error and no audio but it breaks the video preview stream with "The request is invalid in the current state" in the OnEvent callback
				}
			}
		}
	}
	hr = m_pEngineNEW->StartPreview();
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEvent, INFINITE);
	}
	return hr;
}
HRESULT XMFCaptureEngineWrapperRepNEW::StopPreview()
{
	HRESULT hr = m_pEngineNEW->StopPreview();
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEvent, INFINITE);
	}
	return hr;
}
bool XMFCaptureEngineWrapperRepNEW::IsPreviewing() const
{
	return m_bPreviewing;
}
bool XMFCaptureEngineWrapperRepNEW::IsRecording() const
{
	return m_bRecording;
}
HRESULT XMFCaptureEngineWrapperRepNEW::GetStatistics(MF_SINK_WRITER_STATISTICS *pStats)
{
	HRESULT hr = S_OK;
	if (pStats == NULL)
	{
		return E_INVALIDARG;
	}
	if (SUCCEEDED_Xb(hr) && m_pVideoSinkWriterNEW)
	{
		hr = m_pVideoSinkWriterNEW->GetStatistics(m_dwVideoSinkStreamIndex, pStats);
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
HRESULT XMFCaptureEngineWrapperRepNEW::OnEvent(IMFMediaEvent* pEvent)
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
void XMFCaptureEngineWrapperRepNEW::OnCaptureEngineInitialized(HRESULT& hrStatus)
{
	if (hrStatus == MF_E_NO_CAPTURE_DEVICES_AVAILABLE)
	{
		hrStatus = S_OK;  // No capture device. Not an application error.
	}
}
void XMFCaptureEngineWrapperRepNEW::OnPreviewStarted(HRESULT& hrStatus)
{
	m_bPreviewing = SUCCEEDED_Xb(hrStatus);
}
void XMFCaptureEngineWrapperRepNEW::OnPreviewStopped(HRESULT& /*hrStatus*/)
{
	m_bPreviewing = false;
}

void XMFCaptureEngineWrapperRepNEW::OnRecordStarted(HRESULT& hrStatus)
{
	m_bRecording = SUCCEEDED_Xb(hrStatus);
}

void XMFCaptureEngineWrapperRepNEW::OnRecordStopped(HRESULT& /*hrStatus*/)
{
	m_bRecording = false;
}
ULONG XMFCaptureEngineWrapperRepNEW::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}
ULONG XMFCaptureEngineWrapperRepNEW::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}
STDMETHODIMP XMFCaptureEngineWrapperRepNEW::QueryInterface(REFIID iid, void** ppv)
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










class XMFCaptureEngineWrapperRep
{
public:
	XMFCaptureEngineWrapperRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld);
	~XMFCaptureEngineWrapperRep();

	HRESULT StartRecord(PCWSTR pszDestinationFile);
	HRESULT StopRecord();
	
	HRESULT StartPreview(HWND hwnd);
	HRESULT StopPreview();
	
	bool IsPreviewing() const;
	bool IsRecording() const;
	HRESULT get_FramesCaptured(unsigned long* pVal) const;
	HRESULT	get_FPSForCapture(long* pVal) const;

private:
	HRESULT SetupWriter(PCWSTR pszDestinationFile);

	template <class IFACE>
	HRESULT GetCollectionObject(CComPtr<IMFCollection> pCollection, DWORD index, IFACE **ppObject)
	{
		CComPtr<IUnknown> pUnk;
		HRESULT hr = pCollection->GetElement(index, &pUnk);
		if (SUCCEEDED(hr))
		{
			hr = pUnk->QueryInterface(IID_PPV_ARGS(ppObject));
		}
		return hr;
	}

	CComPtr<IMFMediaType> GetAudioEncodingMediaType(CComPtr<IMFMediaType> pAudioInputMediaType);
	CComPtr<IMFMediaType> GetVideoEncodingMediaType(CComPtr<IMFMediaType> pInputMediaType);
	HRESULT CopyAttribute(CComPtr<IMFAttributes> pSrc, CComPtr<IMFAttributes> pDest, const GUID& key);
	HRESULT CreateOutTypeUsingInTypeAttrs(CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFMediaType>& apNewMediaType);
	HRESULT GetEncodingBitrate(CComPtr<IMFMediaType> pMediaType, UINT32* uiEncodingBitrate);

	IXMFCaptureEngine* m_pIXMFCaptureEngine = NULL;
};

XMFCaptureEngineWrapper::XMFCaptureEngineWrapper(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld)
{
	m_pRep = new XMFCaptureEngineWrapperRep(pAudioDevice, pVideoDevice, useOld);
}
XMFCaptureEngineWrapperRep::XMFCaptureEngineWrapperRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld)
{
	if (useOld)
	{
		m_pIXMFCaptureEngine = new XMFCaptureEngineWrapperRepOLD(pAudioDevice, pVideoDevice);
	}
	else
	{
		m_pIXMFCaptureEngine = new XMFCaptureEngineWrapperRepNEW(pAudioDevice, pVideoDevice);
	}
}
XMFCaptureEngineWrapper::~XMFCaptureEngineWrapper()
{
	if (m_pRep)
	{
		delete m_pRep;
	}
}
XMFCaptureEngineWrapperRep::~XMFCaptureEngineWrapperRep()
{
}
HRESULT XMFCaptureEngineWrapperRep::SetupWriter(PCWSTR pszDestinationFile)
{
	HRESULT hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIXMFCaptureEngine->SetupWriter(pszDestinationFile);
	}
	// VIDEO MUST GO FIRST!!
	CComPtr<IMFMediaType> pVideoOutputMediaType;
	if (SUCCEEDED_Xb(hr))
	{
		pVideoOutputMediaType = GetVideoEncodingMediaType(m_pIXMFCaptureEngine->GetVideoMTypeFromSource());
	}
	if (pVideoOutputMediaType)
	{
		m_pIXMFCaptureEngine->AddVideoStream(pVideoOutputMediaType);
	}
	else
	{
		hr = E_FAIL;
	}
	// AUDIO MUST GO SECOND!!
	CComPtr<IMFMediaType> pAudioOutputMediaType;
	if (SUCCEEDED_Xb(hr))
	{
		pAudioOutputMediaType = GetAudioEncodingMediaType(m_pIXMFCaptureEngine->GetAudioMTypeFromSource());
	}
	if (pAudioOutputMediaType)
	{
		m_pIXMFCaptureEngine->AddAudioStream(pAudioOutputMediaType);
	}
	else
	{
		hr = E_FAIL;
	}
	if (SUCCEEDED_Xb(hr))
	{

	}
	return hr;
}
HRESULT XMFCaptureEngineWrapper::StartRecord(PCWSTR pszDestinationFile)
{
	if (m_pRep)
	{
		return m_pRep->StartRecord(pszDestinationFile);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRep::StartRecord(PCWSTR pszDestinationFile)
{
	HRESULT hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = SetupWriter(pszDestinationFile);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIXMFCaptureEngine->StartRecord();
	}
	return hr;
}


HRESULT XMFCaptureEngineWrapper::StopRecord()
{
	if (m_pRep)
	{
		return m_pRep->StopRecord();
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRep::StopRecord()
{
	HRESULT hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIXMFCaptureEngine->StopRecord();
	}
	return hr;
}

HRESULT XMFCaptureEngineWrapper::StartPreview(HWND hwnd)
{
	if (m_pRep)
	{
		return m_pRep->StartPreview(hwnd);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRep::StartPreview(HWND hwnd)
{
	HRESULT hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIXMFCaptureEngine->StartPreview(hwnd);
	}
	return hr;
}

HRESULT XMFCaptureEngineWrapper::StopPreview()
{
	if (m_pRep)
	{
		return m_pRep->StopPreview();
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRep::StopPreview()
{
	HRESULT hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIXMFCaptureEngine->StopPreview();
	}
	return hr;
}
bool XMFCaptureEngineWrapper::IsPreviewing() const
{
	if (m_pRep)
	{
		return m_pRep->IsPreviewing();
	}
	return false;
}
bool XMFCaptureEngineWrapperRep::IsPreviewing() const
{
	return m_pIXMFCaptureEngine->IsPreviewing();
}
bool XMFCaptureEngineWrapper::IsRecording() const
{
	if (m_pRep)
	{
		return m_pRep->IsRecording();
	}
	return false;
}
bool XMFCaptureEngineWrapperRep::IsRecording() const
{
	return m_pIXMFCaptureEngine->IsRecording();
}
CComPtr<IMFMediaType> XMFCaptureEngineWrapperRep::GetAudioEncodingMediaType(CComPtr<IMFMediaType> pAudioInputMediaType)
{
	HRESULT hr = S_OK;
	CComPtr<IMFAttributes> pAttributes = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateAttributes(&pAttributes, 1);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pAttributes->SetUINT32(MF_LOW_LATENCY, TRUE);
	}
	CComPtr<IMFCollection> pAvailableTypes = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = MFTranscodeGetAudioOutputAvailableTypes(MFAudioFormat_AAC, MFT_ENUM_FLAG_ALL | MFT_ENUM_FLAG_SORTANDFILTER, pAttributes, &pAvailableTypes);
	}
	CComPtr<IMFMediaType> retVal = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		// DUMP available AAC formats
		//DWORD dwMTCount = 0;
		//hr = pAvailableTypes->GetElementCount(&dwMTCount);
		//for (DWORD i = 0; i < dwMTCount; i++)
		//{
		//	CComPtr<IMFMediaType> pMediaType = NULL;
		//	hr = GetCollectionObject(pAvailableTypes, i, &pMediaType);
		//	WCHAR count[1024];
		//	swprintf_s(count, 1024, L"%d", i);
		//	DumpAttr(pMediaType, L"AUDIO AAC", count);
		//	OutputDebugStringW(L"\n");
		//}

		// 43 is 
		//MF_MT_AUDIO_SAMPLES_PER_SECOND	48000
		//MF_MT_AUDIO_NUM_CHANNELS			2
		//MF_MT_AVG_BITRATE					192000
		hr = GetCollectionObject(pAvailableTypes, 43, &retVal); // HARD CODED to format 43!!
	}
	if (SUCCEEDED_Xb(hr))
	{
		return retVal;
	}
	return NULL;
}

CComPtr<IMFMediaType> XMFCaptureEngineWrapperRep::GetVideoEncodingMediaType(CComPtr<IMFMediaType> pVideoInputMediaType)
{
	if (pVideoInputMediaType == NULL)
	{
		return NULL;
	}
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> pOutputMediaType = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = CreateOutTypeUsingInTypeAttrs(pVideoInputMediaType, pOutputMediaType);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pOutputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pOutputMediaType->SetUINT32(MF_MT_VIDEO_PROFILE, 100);
		//hr = pOutputMediaType->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_High);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pOutputMediaType->SetUINT32(MF_MT_VIDEO_LEVEL, 41);
		//hr = pOutputMediaType->SetUINT32(MF_MT_MPEG2_LEVEL, eAVEncH264VLevel4_1);
	}
	if (SUCCEEDED_Xb(hr))
	{
		//hr = pOutputMediaType->SetUINT32(MF_MT_MAX_KEYFRAME_SPACING, 30); // ???????????? NOT WORKING!!!!!!!!!!!!!!!!!
		//hr = pOutputMediaType->SetUINT32(CODECAPI_AVEncMPVGOPSize, 30);	// ???????????? NOT WORKING!!!!!!!!!!!!!!!!!
	}
	UINT32 uiEncodingBitrate = 0;
	if (SUCCEEDED_Xb(hr))
	{
		hr = GetEncodingBitrate(pOutputMediaType, &uiEncodingBitrate);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pOutputMediaType->SetUINT32(MF_MT_AVG_BITRATE, uiEncodingBitrate);
	}
	if (SUCCEEDED_Xb(hr))
	{
		return pOutputMediaType;
	}
	return NULL;
}

HRESULT XMFCaptureEngineWrapperRep::CreateOutTypeUsingInTypeAttrs(CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFMediaType>& apNewMediaType)
{
	HRESULT hr = S_OK;

	CComPtr<IMFMediaType> pNewMediaType = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateMediaType(&pNewMediaType);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = pNewMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = CopyAttribute((CComPtr<IMFAttributes>)pInputMediaType, (CComPtr<IMFAttributes>)pNewMediaType, MF_MT_FRAME_SIZE);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = CopyAttribute((CComPtr<IMFAttributes>)pInputMediaType, (CComPtr<IMFAttributes>)pNewMediaType, MF_MT_FRAME_RATE);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = CopyAttribute((CComPtr<IMFAttributes>)pInputMediaType, (CComPtr<IMFAttributes>)pNewMediaType, MF_MT_PIXEL_ASPECT_RATIO);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = CopyAttribute((CComPtr<IMFAttributes>)pInputMediaType, (CComPtr<IMFAttributes>)pNewMediaType, MF_MT_INTERLACE_MODE);
	}

	apNewMediaType = pNewMediaType;

	return hr;
}

HRESULT XMFCaptureEngineWrapperRep::CopyAttribute(CComPtr<IMFAttributes> pSrc, CComPtr<IMFAttributes> pDest, const GUID& key)
{
	PROPVARIANT var;
	PropVariantInit(&var);
	HRESULT hr = pSrc->GetItem(key, &var);
	if (SUCCEEDED_Xb(hr))
	{
		hr = pDest->SetItem(key, var);
		PropVariantClear(&var);
	}
	return hr;
}

HRESULT XMFCaptureEngineWrapperRep::GetEncodingBitrate(CComPtr<IMFMediaType> pMediaType, UINT32* uiEncodingBitrate)
{
	if (!uiEncodingBitrate)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

	//UINT32 uiWidth = 0;
	//UINT32 uiHeight = 0;
	//if (SUCCEEDED_Xb(hr))
	//{
	//	hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight);
	//}

	//UINT32 uiFrameRateNum = 0;
	//UINT32 uiFrameRateDenom = 0;
	//if (SUCCEEDED_Xb(hr))
	//{
	//	hr = MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, &uiFrameRateNum, &uiFrameRateDenom);
	//}

	//float uiBitrate = uiWidth / 3.0f * uiHeight * uiFrameRateNum / uiFrameRateDenom;

	*uiEncodingBitrate = 6000000; // 6 megabits

	return hr;
}

HRESULT XMFCaptureEngineWrapper::get_FramesCaptured(unsigned long* pVal) const
{
	if (m_pRep)
	{
		return m_pRep->get_FramesCaptured(pVal);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRep::get_FramesCaptured(unsigned long* pVal) const
{
	HRESULT hr = S_OK;
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	*pVal = 0;
	MF_SINK_WRITER_STATISTICS stats;
	memset(&stats, 0, sizeof(stats));
	stats.cb = sizeof(stats);
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIXMFCaptureEngine->GetStatistics(&stats);
	}
	if (SUCCEEDED_Xb(hr))
	{
		*pVal = (unsigned long)stats.qwNumSamplesEncoded;
	}
	return hr;
}

HRESULT XMFCaptureEngineWrapper::get_FPSForCapture(long* pVal) const
{
	if (m_pRep)
	{
		return m_pRep->get_FPSForCapture(pVal);
	}
	return E_FAIL;
}
HRESULT	XMFCaptureEngineWrapperRep::get_FPSForCapture(long* pVal) const
{
	HRESULT hr = S_OK;
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	*pVal = 0;
	MF_SINK_WRITER_STATISTICS stats;
	memset(&stats, 0, sizeof(stats));
	stats.cb = sizeof(stats);
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIXMFCaptureEngine->GetStatistics(&stats);
	}
	if (SUCCEEDED_Xb(hr))
	{
		//char mess[255];
		//sprintf_s(mess, 255, "POOP %s long(%d)\n", __FUNCTION__, (long)stats.dwAverageSampleRateReceived * 100);
		//OutputDebugStringA(mess);

		*pVal = (long)stats.dwAverageSampleRateReceived * 100;
	}
	return hr;
}