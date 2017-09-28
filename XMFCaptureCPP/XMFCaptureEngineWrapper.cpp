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

class XMFCaptureEngineWrapperRep
{
public:
	XMFCaptureEngineWrapperRep(IMFCaptureEngineOnEventCallback *pEventCallback, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, HANDLE hEvent, bool useOld);
	~XMFCaptureEngineWrapperRep();

	HRESULT SetupWriter(PCWSTR pszDestinationFile);

	HRESULT StartRecord();
	HRESULT StopRecord();
	
	HRESULT StartPreview(HWND hwnd);
	HRESULT StopPreview();
	
	HRESULT get_FramesCaptured(unsigned long* pVal) const;
	HRESULT	get_FPSForCapture(long* pVal) const;

private:
	CComPtr<IMFMediaType> GetAudioMTypeFromSource();
	CComPtr<IMFMediaType> GetVideoMTypeFromSource();

	// new
	CComPtr<IMFCaptureSink> GetPreviewSinkNEW();
	CComPtr<IMFCaptureSource> GetCaptureSourceNEW();

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

	DWORD								m_dwVideoSinkStreamIndex;
	DWORD								m_dwAudioSinkStreamIndex;

	CComPtr<IMFMediaType> GetAudioEncodingMediaType(CComPtr<IMFMediaType> pAudioInputMediaType);
	CComPtr<IMFMediaType> GetVideoEncodingMediaType(CComPtr<IMFMediaType> pInputMediaType);
	HRESULT CopyAttribute(CComPtr<IMFAttributes> pSrc, CComPtr<IMFAttributes> pDest, const GUID& key);
	HRESULT CreateOutTypeUsingInTypeAttrs(CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFMediaType>& apNewMediaType);
	HRESULT GetEncodingBitrate(CComPtr<IMFMediaType> pMediaType, UINT32* uiEncodingBitrate);

	bool mUseOld;
	// new
	CComPtr<IMFCaptureEngine>			m_pEngineNEW;
	HANDLE								m_hEventNEW;
	CComPtr<IMFSinkWriter>				m_pVideoSinkWriterNEW;
	CComPtr<IMFCapturePreviewSink>		m_pPreviewNEW;
	CComPtr<IMFCaptureSink> GetCaptureSinkNEW();

	// old
	CComPtr<IMFMediaSource>	m_pAggregatSourceOLD;
	XMFAVSourceReader* m_pXMFAVSourceReaderOLD;
	XMFSinkWriter* m_pXMFSinkWriterOLD = NULL;
	CComPtr<IMFMediaSource> GetAggregateMediaSourceOLD(CComPtr<IMFMediaSource> pAudioSource, CComPtr<IMFMediaSource> pVideoSource);
};

XMFCaptureEngineWrapper::XMFCaptureEngineWrapper(IMFCaptureEngineOnEventCallback *pEventCallback, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, HANDLE hEvent, bool useOld)
{
	m_pRep = new XMFCaptureEngineWrapperRep(pEventCallback, pAudioDevice, pVideoDevice, hEvent, useOld);
}
XMFCaptureEngineWrapperRep::XMFCaptureEngineWrapperRep(IMFCaptureEngineOnEventCallback *pEventCallback, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, HANDLE hEvent, bool useOld) :
	mUseOld(useOld),
	m_pEngineNEW(NULL),
	m_hEventNEW(hEvent),
	m_pAggregatSourceOLD(NULL),
	m_pXMFAVSourceReaderOLD(NULL),
	m_pXMFSinkWriterOLD(NULL),
	m_dwVideoSinkStreamIndex(0),
	m_dwAudioSinkStreamIndex(0),
	m_pVideoSinkWriterNEW(NULL),
	m_pPreviewNEW(NULL)
{
	HRESULT hr = S_OK;
	CComPtr<IMFCaptureEngineClassFactory> pFactory = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = CoCreateInstance(CLSID_MFCaptureEngineClassFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
	}

	if (mUseOld)
	{
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
	else
	{
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
			hr = m_pEngineNEW->Initialize(pEventCallback, pAttributes, pAudioActivate, pVideoActivate);
			WaitForSingleObject(m_hEventNEW, INFINITE);
		}
	}
	SUCCEEDED_Xv(hr);
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

CComPtr<IMFCaptureSink> XMFCaptureEngineWrapperRep::GetCaptureSinkNEW()
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

CComPtr<IMFCaptureSink> XMFCaptureEngineWrapperRep::GetPreviewSinkNEW()
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

HRESULT XMFCaptureEngineWrapper::SetupWriter(PCWSTR pszDestinationFile)
{
	if (m_pRep)
	{
		return m_pRep->SetupWriter(pszDestinationFile);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRep::SetupWriter(PCWSTR pszDestinationFile)
{
	HRESULT hr = S_OK;
	CComPtr<IMFCaptureRecordSink> pCaptureRecordSinkNEW = NULL;
	CComPtr<IMFCaptureSource> pCaptureSourceNEW = NULL;
	if (mUseOld)
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
	}
	else
	{
		if (SUCCEEDED_Xb(hr))
		{
			hr = GetCaptureSinkNEW()->QueryInterface(IID_PPV_ARGS(&pCaptureRecordSinkNEW));
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
			hr = pCaptureRecordSinkNEW->RemoveAllStreams();
		}

		if (SUCCEEDED_Xb(hr))
		{
			hr = pCaptureRecordSinkNEW->SetOutputFileName(pszDestinationFile);
		}
	}

	// VIDEO MUST GO FIRST!!
	CComPtr<IMFMediaType> pVideoOutputMediaType;
	if (SUCCEEDED_Xb(hr))
	{
		pVideoOutputMediaType = GetVideoEncodingMediaType(GetVideoMTypeFromSource());
	}
	if (pVideoOutputMediaType)
	{
		if (pCaptureRecordSinkNEW)
		{
			hr = pCaptureRecordSinkNEW->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_VIDEO_RECORD, pVideoOutputMediaType, NULL, &m_dwVideoSinkStreamIndex);
		}
		else if (m_pXMFSinkWriterOLD)
		{
			hr = m_pXMFSinkWriterOLD->AddStream(pVideoOutputMediaType, &m_dwVideoSinkStreamIndex);
			if (SUCCEEDED_Xb(hr))
			{
				hr = m_pXMFSinkWriterOLD->SetInputMediaType(m_dwVideoSinkStreamIndex, GetVideoMTypeFromSource(), NULL);
			}
		}
		else
		{
			hr = E_FAIL;
		}
	}
	else
	{
		hr = E_FAIL;
	}

	// AUDIO MUST GO SECOND!!
	CComPtr<IMFMediaType> pAudioOutputMediaType;
	if (SUCCEEDED_Xb(hr))
	{
		pAudioOutputMediaType = GetAudioEncodingMediaType(GetAudioMTypeFromSource());
	}
	if (pAudioOutputMediaType)
	{
		if (pCaptureRecordSinkNEW)
		{
			hr = pCaptureRecordSinkNEW->AddStream((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO, pAudioOutputMediaType, NULL, &m_dwAudioSinkStreamIndex);
			if (hr == MF_E_INVALIDSTREAMNUMBER)
			{
				//If an audio device is not present, allow video only recording
				hr = S_OK;
			}
		}
		else if (m_pXMFSinkWriterOLD)
		{
			hr = m_pXMFSinkWriterOLD->AddStream(pAudioOutputMediaType, &m_dwAudioSinkStreamIndex);
			if (SUCCEEDED_Xb(hr))
			{
				hr = m_pXMFSinkWriterOLD->SetInputMediaType(m_dwAudioSinkStreamIndex, GetAudioMTypeFromSource(), NULL);
			}
		}
		else
		{
			hr = E_FAIL;
		}
	}
	else
	{
		hr = E_FAIL;
	}

	if (SUCCEEDED_Xb(hr))
	{
		if (m_pXMFSinkWriterOLD)
		{
			hr = m_pXMFSinkWriterOLD->BeginWriting();
		}
	}
	return hr;
}

CComPtr<IMFCaptureSource> XMFCaptureEngineWrapperRep::GetCaptureSourceNEW()
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

HRESULT XMFCaptureEngineWrapper::StartRecord()
{
	if (m_pRep)
	{
		return m_pRep->StartRecord();
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineWrapperRep::StartRecord()
{
	HRESULT hr = S_OK;
	if (m_pEngineNEW)
	{
		hr = m_pEngineNEW->StartRecord();
		if (SUCCEEDED_Xb(hr))
		{
			WaitForSingleObject(m_hEventNEW, INFINITE);
		}
	}
	else if (m_pXMFAVSourceReaderOLD)
	{
		hr = m_pXMFAVSourceReaderOLD->Start();
	}
	else
	{
		hr = E_FAIL;
	}
	if (!mUseOld)
	{
		if (SUCCEEDED_Xb(hr))
		{
			hr = GetCaptureSinkNEW()->GetService(m_dwVideoSinkStreamIndex, GUID_NULL, IID_IMFSinkWriter, (IUnknown**)&m_pVideoSinkWriterNEW);
		}
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
	if (m_pEngineNEW)
	{
		hr = m_pEngineNEW->StopRecord(TRUE, TRUE);
		if (SUCCEEDED_Xb(hr))
		{
			WaitForSingleObject(m_hEventNEW, INFINITE);
		}
	}
	else if (m_pXMFAVSourceReaderOLD)
	{
		if (m_pXMFSinkWriterOLD)
		{
			hr = m_pXMFSinkWriterOLD->EndWriting();
		}
		delete m_pXMFAVSourceReaderOLD;
		m_pXMFAVSourceReaderOLD = NULL;
	}
	else
	{
		hr = E_FAIL;
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
		WaitForSingleObject(m_hEventNEW, INFINITE);
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
	HRESULT hr = m_pEngineNEW->StopPreview();
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEventNEW, INFINITE);
	}
	return hr;
}

CComPtr<IMFMediaSource> XMFCaptureEngineWrapperRep::GetAggregateMediaSourceOLD(CComPtr<IMFMediaSource> pAudioSource, CComPtr<IMFMediaSource> pVideoSource)
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

CComPtr<IMFMediaType> XMFCaptureEngineWrapperRep::GetAudioMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	if (mUseOld)
	{
		if (m_pXMFAVSourceReaderOLD)
		{
			hr = m_pXMFAVSourceReaderOLD->GetAudioInputMediaType(retVal);
			if (SUCCEEDED_Xb(hr))
			{
				return retVal;
			}
		}
	}
	else
	{
		CComPtr<IMFCaptureSource> pSource = GetCaptureSourceNEW();
		if (pSource)
		{
			hr = pSource->GetAvailableDeviceMediaType((DWORD)MF_CAPTURE_ENGINE_PREFERRED_SOURCE_STREAM_FOR_AUDIO, 0, &retVal);
		}
		if (SUCCEEDED_Xb(hr))
		{
			return retVal;
		}
	}
	return NULL;
}

CComPtr<IMFMediaType> XMFCaptureEngineWrapperRep::GetVideoMTypeFromSource()
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaType> retVal = NULL;
	if (mUseOld)
	{
		if (m_pXMFAVSourceReaderOLD)
		{
			hr = m_pXMFAVSourceReaderOLD->GetVideoInputMediaType(retVal);
			if (SUCCEEDED_Xb(hr))
			{
				return retVal;
			}
		}
	}
	else
	{
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
	}
	return NULL;
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
	if (SUCCEEDED_Xb(hr) && m_pVideoSinkWriterNEW)
	{
		hr = m_pVideoSinkWriterNEW->GetStatistics(m_dwVideoSinkStreamIndex, &stats);
	}
	else if (SUCCEEDED_Xb(hr) && m_pXMFSinkWriterOLD)
	{
		m_pXMFSinkWriterOLD->GetStatistics(m_dwVideoSinkStreamIndex, &stats);
	}
	else
	{
		hr = E_FAIL;
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
	if (SUCCEEDED_Xb(hr) && m_pVideoSinkWriterNEW)
	{
		hr = m_pVideoSinkWriterNEW->GetStatistics(m_dwVideoSinkStreamIndex, &stats);
	}
	else if (SUCCEEDED_Xb(hr) && m_pXMFSinkWriterOLD)
	{
		m_pXMFSinkWriterOLD->GetStatistics(m_dwVideoSinkStreamIndex, &stats);
	}
	else
	{
		hr = E_FAIL;
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