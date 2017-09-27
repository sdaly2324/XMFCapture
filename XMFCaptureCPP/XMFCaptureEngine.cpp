#include "stdafx.h"

#include "XMFCaptureEngine.h"
#include "XMFUtilities.h"
#include "XMFCaptureDevice.h"
#include "XMFAVSourceReader.h"
#include "XMFSinkWriter.h"

#include <windows.h>
#include <memory>
#include <Mferror.h>
#include <winerror.h>
#include <Mfcaptureengine.h>
#include <combaseapi.h>
#include <codecapi.h>

// old
#include "XMFSinkWriterCallback.h"	
#include "XMFSourceReaderCallback.h"

#define IDS_ERR_INITIALIZE              104
#define IDS_ERR_PREVIEW                 105
#define IDS_ERR_RECORD                  106
#define IDS_ERR_CAPTURE                 107


class CaptureEngineWrapper
{
public:
	CaptureEngineWrapper(IMFCaptureEngineOnEventCallback *pEventCallback, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, HANDLE hEvent, bool useOld);
	~CaptureEngineWrapper();

	HRESULT StartRecord();
	HRESULT StopRecord();
	HRESULT StartPreview();
	HRESULT StopPreview();
	CComPtr<IMFMediaType> GetAudioMTypeFromSource();
	CComPtr<IMFMediaType> GetVideoMTypeFromSource();
	HRESULT SetupWriter(PCWSTR pszDestinationFile);
	HRESULT get_FramesCaptured(unsigned long* pVal) const;
	HRESULT	get_FPSForCapture(long* pVal) const;

	// new
	CComPtr<IMFCaptureSink> GetPreviewSinkNEW();
	CComPtr<IMFCaptureSource> GetCaptureSourceNEW();

private:

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
	CComPtr<IMFCaptureSink> GetCaptureSinkNEW();

	// old
	CComPtr<IMFMediaSource>	m_pAggregatSourceOLD;
	XMFAVSourceReader* m_pXMFAVSourceReaderOLD;
	XMFSinkWriter* m_pXMFSinkWriterOLD = NULL;
	CComPtr<IMFMediaSource> GetAggregateMediaSourceOLD(CComPtr<IMFMediaSource> pAudioSource, CComPtr<IMFMediaSource> pVideoSource);
};

CaptureEngineWrapper::CaptureEngineWrapper(IMFCaptureEngineOnEventCallback *pEventCallback, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, HANDLE hEvent, bool useOld):
	mUseOld(useOld),
	m_pEngineNEW(NULL),
	m_hEventNEW(hEvent),
	m_pAggregatSourceOLD(NULL),
	m_pXMFAVSourceReaderOLD(NULL),
	m_pXMFSinkWriterOLD(NULL),
	m_dwVideoSinkStreamIndex(0),
	m_dwAudioSinkStreamIndex(0),
	m_pVideoSinkWriterNEW(NULL)
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

CaptureEngineWrapper::~CaptureEngineWrapper()
{
}

CComPtr<IMFCaptureSink> CaptureEngineWrapper::GetCaptureSinkNEW()
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

CComPtr<IMFCaptureSink> CaptureEngineWrapper::GetPreviewSinkNEW()
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

HRESULT CaptureEngineWrapper::SetupWriter(PCWSTR pszDestinationFile)
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

CComPtr<IMFCaptureSource> CaptureEngineWrapper::GetCaptureSourceNEW()
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

HRESULT CaptureEngineWrapper::StartRecord()
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

HRESULT CaptureEngineWrapper::StopRecord()
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
HRESULT CaptureEngineWrapper::StartPreview()
{
	HRESULT hr = m_pEngineNEW->StartPreview();
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEventNEW, INFINITE);
	}
	return hr;
}
HRESULT CaptureEngineWrapper::StopPreview()
{
	HRESULT hr = m_pEngineNEW->StopPreview();
	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEventNEW, INFINITE);
	}
	return hr;
}

CComPtr<IMFMediaSource> CaptureEngineWrapper::GetAggregateMediaSourceOLD(CComPtr<IMFMediaSource> pAudioSource, CComPtr<IMFMediaSource> pVideoSource)
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

CComPtr<IMFMediaType> CaptureEngineWrapper::GetAudioMTypeFromSource()
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
CComPtr<IMFMediaType> CaptureEngineWrapper::GetVideoMTypeFromSource()
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

CComPtr<IMFMediaType> CaptureEngineWrapper::GetAudioEncodingMediaType(CComPtr<IMFMediaType> pAudioInputMediaType)
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

CComPtr<IMFMediaType> CaptureEngineWrapper::GetVideoEncodingMediaType(CComPtr<IMFMediaType> pVideoInputMediaType)
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

HRESULT CaptureEngineWrapper::CreateOutTypeUsingInTypeAttrs(CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFMediaType>& apNewMediaType)
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

HRESULT CaptureEngineWrapper::CopyAttribute(CComPtr<IMFAttributes> pSrc, CComPtr<IMFAttributes> pDest, const GUID& key)
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

HRESULT CaptureEngineWrapper::GetEncodingBitrate(CComPtr<IMFMediaType> pMediaType, UINT32* uiEncodingBitrate)
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

HRESULT CaptureEngineWrapper::get_FramesCaptured(unsigned long* pVal) const
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

HRESULT	CaptureEngineWrapper::get_FPSForCapture(long* pVal) const
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








class XMFCaptureEngineRep : public IMFCaptureEngineOnEventCallback, public IMFCaptureEngineOnSampleCallback2
{
public:
	XMFCaptureEngineRep(HWND hwnd, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld);
	~XMFCaptureEngineRep();

	HRESULT StartPreview();
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

	CaptureEngineWrapper*				m_pCaptureEngineWrapper;
	
	CComPtr<IMFCapturePreviewSink>		m_pPreview;

	HWND								m_hwndPreview;
	HANDLE								m_hEvent;

	bool								m_bRecording;
	bool								m_bPreviewing;
	UINT								m_errorID;
	HANDLE								m_hpwrRequest;
	bool								m_fPowerRequestSet;
	bool								m_fSleeping;
	CComAutoCriticalSection				m_criticalSection;
};

XMFCaptureEngine::XMFCaptureEngine(HWND hwnd, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld)
{
	m_RepPtr = new XMFCaptureEngineRep(hwnd, pAudioDevice, pVideoDevice, useOld);
}
XMFCaptureEngineRep::XMFCaptureEngineRep(HWND hwnd, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld) :
	m_pCaptureEngineWrapper(NULL),
	m_pPreview(NULL),
	m_hEvent(NULL),
	m_hwndPreview(hwnd),
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
	m_pCaptureEngineWrapper = new CaptureEngineWrapper(this, pAudioDevice, pVideoDevice, m_hEvent, useOld);
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

HRESULT XMFCaptureEngine::StartPreview()
{
	if (m_RepPtr)
	{
		return m_RepPtr->StartPreview();
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::StartPreview()
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

	if (m_pPreview == NULL)
	{
		CComPtr<IMFCaptureSink> pPreviewSink = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			pPreviewSink = m_pCaptureEngineWrapper->GetPreviewSinkNEW();
			if (pPreviewSink == NULL)
			{
				return E_FAIL;
			}
		}
		if (SUCCEEDED_Xb(hr))
		{
			hr = pPreviewSink->QueryInterface(IID_PPV_ARGS(&m_pPreview));
		}
		CComPtr<IMFCaptureSource> pPreviewSource = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			pPreviewSource = m_pCaptureEngineWrapper->GetCaptureSourceNEW();
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
					hr = m_pPreview->SetRenderHandle(m_hwndPreview);
					hr = m_pPreview->AddStream(i, pMediaType, NULL, &dwSinkStreamIndex);
				}
				else if (guidValue == MFMediaType_Audio)
				{
					//hr = m_pPreview->AddStream(i, pMediaType, NULL, &dwSinkStreamIndex); // not only does this give you no hr error and no audio but it breaks the video preview stream with "The request is invalid in the current state" in the OnEvent callback
				}
			}
		}
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureEngineWrapper->StartPreview();
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

	if (SUCCEEDED_Xb(hr))
	{
		WaitForSingleObject(m_hEvent, INFINITE);
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