#include "stdafx.h"
#include "XMFCaptureEngineWrapper.h"

#include <Mferror.h>
#include <codecapi.h>

#include "XMFCaptureUsingIMFSinkWriter.h"
#include "XMFCaptureUsingIMFCaptureEngine.h"

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
		m_pIXMFCaptureEngine = new XMFCaptureUsingIMFSinkWriter(pAudioDevice, pVideoDevice);
	}
	else
	{
		m_pIXMFCaptureEngine = new XMFCaptureUsingIMFCaptureEngine(pAudioDevice, pVideoDevice);
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
	PWSTR pszExt = PathFindExtension(pszDestinationFile);
	if (!(_wcsicmp(pszExt, L".mp4") == 0 || _wcsicmp(pszExt, L".ts") == 0))
	{
		return MF_E_INVALIDMEDIATYPE;
	}
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
		//hr = pOutputMediaType->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_High); // BOTH WORK
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pOutputMediaType->SetUINT32(MF_MT_VIDEO_LEVEL, 41);
		//hr = pOutputMediaType->SetUINT32(MF_MT_MPEG2_LEVEL, eAVEncH264VLevel4_1); // BOTH WORK
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pOutputMediaType->SetUINT32(MF_MT_MAX_KEYFRAME_SPACING, 30); // ???????????? NOT WORKING!!!!!!!!!!!!!!!!!
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