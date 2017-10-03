#include "stdafx.h"
#include "XMFCaptureUsingIMFSinkWriter.h"

#include <Mferror.h>
#include <mfcaptureengine.h>

#include "XMFCaptureDevice.h"
#include "XMFSinkWriter.h"
#include "XMFAVSourceReader.h"

class XMFCaptureUsingIMFSinkWriterRep : public IXMFCaptureEngine, public IMFCaptureEngineOnSampleCallback2
{
public:
	XMFCaptureUsingIMFSinkWriterRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice);
	~XMFCaptureUsingIMFSinkWriterRep();

	HRESULT SetupWriter(PCWSTR pszDestinationFile);
	CComPtr<IMFMediaType> GetAudioMTypeFromSource();
	CComPtr<IMFMediaType> GetVideoMTypeFromSource();
	HRESULT AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType);
	HRESULT AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType);

	HRESULT StartRecord(HWND hwnd);
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
	CComPtr<IMFMediaSource> GetAggregateMediaSource(CComPtr<IMFMediaSource> pAudioSource, CComPtr<IMFMediaSource> pVideoSource);

	// IMFCaptureEngineOnSampleCallback2
	STDMETHODIMP OnSample(IMFSample *pSample);
	STDMETHODIMP OnSynchronizedEvent(IMFMediaEvent* pEvent);

	CComPtr<IMFMediaSource>	m_pAggregatSource = NULL;
	XMFAVSourceReader* m_pXMFAVSourceReader = NULL;
	XMFSinkWriter* m_pXMFSinkWriter = NULL;
	bool m_bRecording = false;
	const UINT IDS_ERR_INITIALIZE = 104;
	const UINT IDS_ERR_PREVIEW = 105;
	const UINT IDS_ERR_RECORD = 106;
	const UINT IDS_ERR_CAPTURE = 107;
	DWORD m_dwVideoSinkStreamIndex = 0;
};

XMFCaptureUsingIMFSinkWriter::XMFCaptureUsingIMFSinkWriter(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice)
{
	m_pRep = new XMFCaptureUsingIMFSinkWriterRep(pAudioDevice, pVideoDevice);
}
XMFCaptureUsingIMFSinkWriterRep::XMFCaptureUsingIMFSinkWriterRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice)
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaSource> pAudioSource = NULL;
	if (SUCCEEDED_Xb(hr) && pAudioDevice)
	{
		hr = pAudioDevice->GetIMFMediaSource(pAudioSource);
		m_pAggregatSource = pAudioSource;
	}
	CComPtr<IMFMediaSource> pVideoSource = NULL;
	if (SUCCEEDED_Xb(hr) && pVideoDevice)
	{
		hr = pVideoDevice->GetIMFMediaSource(pVideoSource);
		m_pAggregatSource = pVideoSource;
	}
	if (SUCCEEDED_Xb(hr) && pAudioSource && pVideoSource)
	{
		m_pAggregatSource = GetAggregateMediaSource(pAudioSource, pVideoSource);
	}
}
XMFCaptureUsingIMFSinkWriter::~XMFCaptureUsingIMFSinkWriter()
{
	if (m_pRep)
	{
		delete m_pRep;
	}
}
XMFCaptureUsingIMFSinkWriterRep::~XMFCaptureUsingIMFSinkWriterRep()
{

}
HRESULT XMFCaptureUsingIMFSinkWriter::SetupWriter(PCWSTR pszDestinationFile)
{
	if (m_pRep)
	{
		return m_pRep->SetupWriter(pszDestinationFile);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::SetupWriter(PCWSTR pszDestinationFile)
{
	if (m_pXMFSinkWriter)
	{
		delete m_pXMFSinkWriter;
	}
	m_pXMFSinkWriter = new XMFSinkWriter(pszDestinationFile);
	if (m_pXMFSinkWriter)
	{
		if (m_pXMFAVSourceReader)
		{
			delete m_pXMFAVSourceReader;
		}
		m_pXMFAVSourceReader = new XMFAVSourceReader(m_pXMFSinkWriter, m_pAggregatSource);
	}
	return S_OK;
}
CComPtr<IMFMediaType> XMFCaptureUsingIMFSinkWriter::GetAudioMTypeFromSource()
{
	if (m_pRep)
	{
		return m_pRep->GetAudioMTypeFromSource();
	}
	return NULL;
}
CComPtr<IMFMediaType> XMFCaptureUsingIMFSinkWriterRep::GetAudioMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	if (m_pXMFAVSourceReader)
	{
		hr = m_pXMFAVSourceReader->GetAudioInputMediaType(retVal);
		if (SUCCEEDED_Xb(hr))
		{
			return retVal;
		}
	}
	return NULL;
}
CComPtr<IMFMediaType> XMFCaptureUsingIMFSinkWriter::GetVideoMTypeFromSource()
{
	if (m_pRep)
	{
		return m_pRep->GetVideoMTypeFromSource();
	}
	return NULL;
}
CComPtr<IMFMediaType> XMFCaptureUsingIMFSinkWriterRep::GetVideoMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	if (m_pXMFAVSourceReader)
	{
		hr = m_pXMFAVSourceReader->GetVideoInputMediaType(retVal);
		if (SUCCEEDED_Xb(hr))
		{
			return retVal;
		}
	}
	return NULL;
}
HRESULT XMFCaptureUsingIMFSinkWriter::AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType)
{
	if (m_pRep)
	{
		return m_pRep->AddVideoStream(pVideoOutputMediaType);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType)
{
	if (m_pXMFSinkWriter == NULL)
	{
		return E_FAIL;
	}
	HRESULT  hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pXMFSinkWriter->AddStream(pVideoOutputMediaType, &m_dwVideoSinkStreamIndex);
	}
	CComPtr<IMFMediaType> pInputMediaType = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		m_pXMFAVSourceReader->GetVideoInputMediaType(pInputMediaType);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pXMFSinkWriter->SetInputMediaType(m_dwVideoSinkStreamIndex, pInputMediaType, NULL);
	}
	return hr;
}
HRESULT XMFCaptureUsingIMFSinkWriter::AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType)
{
	if (m_pRep)
	{
		return m_pRep->AddAudioStream(pAudioOutputMediaType);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType)
{
	if (m_pXMFSinkWriter == NULL)
	{
		return E_FAIL;
	}
	HRESULT  hr = S_OK;
	DWORD aideoSinkStreamIndex = 0;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pXMFSinkWriter->AddStream(pAudioOutputMediaType, &aideoSinkStreamIndex);
	}
	CComPtr<IMFMediaType> pInputMediaType = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		m_pXMFAVSourceReader->GetAudioInputMediaType(pInputMediaType);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pXMFSinkWriter->SetInputMediaType(aideoSinkStreamIndex, pInputMediaType, NULL);
	}
	return hr;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::OnSynchronizedEvent(IMFMediaEvent* pEvent)
{
	OutputDebugStringA("Sample format change!\n");

	HRESULT hr = S_OK;

	if (pEvent == NULL)
	{
		return E_INVALIDARG;
	}

	return hr;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::OnSample(IMFSample *pSample)
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
HRESULT XMFCaptureUsingIMFSinkWriter::StartRecord(HWND hwnd)
{
	if (m_pRep)
	{
		return m_pRep->StartRecord(hwnd);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::StartRecord(HWND /*hwnd*/)
{
	HRESULT hr = S_OK;
	if (m_bRecording == true)
	{
		return MF_E_INVALIDREQUEST;
	}
	if (m_pXMFSinkWriter)
	{
		hr = m_pXMFSinkWriter->BeginWriting();
	}
	if (SUCCEEDED_Xb(hr) && m_pXMFAVSourceReader)
	{
		hr = m_pXMFAVSourceReader->Start();
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
HRESULT XMFCaptureUsingIMFSinkWriter::StopRecord()
{
	if (m_pRep)
	{
		return m_pRep->StopRecord();
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::StopRecord()
{
	HRESULT hr = S_OK;
	if (m_pXMFSinkWriter)
	{
		hr = m_pXMFSinkWriter->EndWriting();
	}
	delete m_pXMFAVSourceReader;
	m_pXMFAVSourceReader = NULL;
	return hr;
}
HRESULT XMFCaptureUsingIMFSinkWriter::StartPreview(HWND hwnd)
{
	if (m_pRep)
	{
		return m_pRep->StartPreview(hwnd);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::StartPreview(HWND /*hwnd*/)
{
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFSinkWriter::StopPreview()
{
	if (m_pRep)
	{
		return m_pRep->StopPreview();
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::StopPreview()
{
	return E_FAIL;
}
bool XMFCaptureUsingIMFSinkWriter::IsPreviewing() const
{
	if (m_pRep)
	{
		return m_pRep->IsPreviewing();
	}
	return false;
}
bool XMFCaptureUsingIMFSinkWriterRep::IsPreviewing() const
{
	return false;
}
bool XMFCaptureUsingIMFSinkWriter::IsRecording() const
{
	if (m_pRep)
	{
		return m_pRep->IsRecording();
	}
	return false;
}
bool XMFCaptureUsingIMFSinkWriterRep::IsRecording() const
{
	return m_bRecording;
}
HRESULT XMFCaptureUsingIMFSinkWriter::GetStatistics(MF_SINK_WRITER_STATISTICS* pStats)
{
	if (m_pRep)
	{
		return m_pRep->GetStatistics(pStats);
	}
	return E_FAIL;
}
HRESULT XMFCaptureUsingIMFSinkWriterRep::GetStatistics(MF_SINK_WRITER_STATISTICS *pStats)
{
	HRESULT hr = S_OK;
	if (pStats == NULL)
	{
		return E_INVALIDARG;
	}
	if (SUCCEEDED_Xb(hr) && m_pXMFSinkWriter)
	{
		m_pXMFSinkWriter->GetStatistics(m_dwVideoSinkStreamIndex, pStats);
	}
	return hr;
}
CComPtr<IMFMediaSource> XMFCaptureUsingIMFSinkWriterRep::GetAggregateMediaSource(CComPtr<IMFMediaSource> pAudioSource, CComPtr<IMFMediaSource> pVideoSource)
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
ULONG XMFCaptureUsingIMFSinkWriterRep::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}
ULONG XMFCaptureUsingIMFSinkWriterRep::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}
STDMETHODIMP XMFCaptureUsingIMFSinkWriterRep::QueryInterface(REFIID iid, void** ppv)
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
