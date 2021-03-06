#include "XMFAVSourceReader.h"
#include "XMFSourceReaderCallback.h"
#include "XMFUtilities.h"
#include "XMFSinkWriter.h"
#include <atlbase.h>

class XMFAVSourceReaderRep
{
public:
	friend class XMFSourceReaderCallback;
	XMFAVSourceReaderRep(XMFSinkWriter* pXMFSinkWriter, CComPtr<IMFMediaSource> pVASource, CComPtr<IMFAttributes> pMFAttributes);
	virtual ~XMFAVSourceReaderRep();

	HRESULT Start();
	HRESULT GetVideoInputMediaType(CComPtr<IMFMediaType>& pVideoReaderInputMediaTypeCurrent);
	HRESULT GetAudioInputMediaType(CComPtr<IMFMediaType>& pAudioInputMediaType);
	HRESULT GetPresentationDescriptor(CComPtr<IMFPresentationDescriptor>& pVideoInputPresentationDescriptor);
	HRESULT GetMFMediaSource(CComPtr<IMFMediaSource>& pMFMediaSource);
	bool Capturing();

	HRESULT OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimeStamp, CComPtr<IMFSample> pMFSample);
	HRESULT OnFlush(DWORD dwStreamIndex);
private:
	XMFAVSourceReaderRep();

	HRESULT ConfigureVASourceReaderVideo();
	HRESULT ConfigureVASourceReaderAudio();

	CComPtr<IMFMediaSource>				m_pVASource;
	CComPtr<IMFSourceReader>			m_pVASourceReader;
	CComPtr<IMFPresentationDescriptor>	m_pMFPresentationDescriptor;

	XMFSinkWriter*		m_pXMFSinkWriter;

	bool			m_bFirstSample;
	LONGLONG		m_llBaseTime;

	bool			m_bCapturePumpIsRunning;

#ifdef DEBUG_TIMMING
	__int64 m_startStream0;
	__int64 m_startStream1;
	__int64 m_freq;
	int m_count0;
	int m_count1;
#endif
};

XMFAVSourceReader::XMFAVSourceReader(XMFSinkWriter* pXMFSinkWriter, CComPtr<IMFMediaSource> pVASource)
{
	HRESULT hr = S_OK;

	CComPtr<IMFAttributes> pMFAttributes = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateAttributes(&pMFAttributes, 0);
	}

	if (SUCCEEDED_Xb(hr) && pMFAttributes)
	{
		// for some reason SetUnknown does not count towards the IMFAttributes count
		hr = pMFAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, new XMFSourceReaderCallback(this));
	}

	m_pRep = new XMFAVSourceReaderRep(pXMFSinkWriter, pVASource, pMFAttributes);
}

XMFAVSourceReaderRep::XMFAVSourceReaderRep(XMFSinkWriter* pXMFSinkWriter, CComPtr<IMFMediaSource> pVASource, CComPtr<IMFAttributes> pMFAttributes) :
	m_pVASource(pVASource),
	m_pXMFSinkWriter(pXMFSinkWriter),
	m_pVASourceReader(NULL),
	m_pMFPresentationDescriptor(NULL),
	m_bFirstSample(false),
	m_llBaseTime(0),
	m_bCapturePumpIsRunning(false)
#ifdef DEBUG_TIMMING
	, m_startStream0(0),
	m_startStream1(0),
	m_freq(0),
	m_count0(0),
	m_count1(0)
#endif
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateSourceReaderFromMediaSource(m_pVASource, pMFAttributes, &m_pVASourceReader);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = ConfigureVASourceReaderVideo();
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = ConfigureVASourceReaderAudio();
	}
}

XMFAVSourceReader::~XMFAVSourceReader()
{
	if (m_pRep)
	{
		delete m_pRep;
	}
}
XMFAVSourceReaderRep::~XMFAVSourceReaderRep()
{
	if (m_pXMFSinkWriter)
	{
		delete m_pXMFSinkWriter;
	}
	m_pXMFSinkWriter = NULL;

	if (m_pVASource)
	{
		m_pVASource->Shutdown();
		//delete m_pVASource;	// crashing....leak?
	}
	//m_pVASource = NULL;

	if (m_pVASourceReader)
	{
		//delete m_pVASourceReader;	// crashing....leak?
	}
	//m_pVASourceReader = NULL;

	if (m_pMFPresentationDescriptor)
	{
		delete m_pMFPresentationDescriptor;
	}
	m_pVASourceReader = NULL;
}

HRESULT XMFAVSourceReader::OnFlush(DWORD dwStreamIndex)
{
	if (m_pRep)
	{
		return m_pRep->OnFlush(dwStreamIndex);
	}
	return E_FAIL;
}
HRESULT XMFAVSourceReaderRep::OnFlush(DWORD dwStreamIndex)
{
	WCHAR mess[1024];
	swprintf_s(mess, 1024, L"XMFAVSourceReaderRep::OnFlush stream(%d)\n", dwStreamIndex);
	OutputDebugStringW(mess);

	if (m_pXMFSinkWriter)
	{
		return m_pXMFSinkWriter->SignalAllStopped();
	}
	return E_FAIL;
}

HRESULT XMFAVSourceReader::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimeStamp, CComPtr<IMFSample> pMFSample)
{
	if (m_pRep)
	{
		return m_pRep->OnReadSample(hrStatus, dwStreamIndex, dwStreamFlags, llTimeStamp, pMFSample);
	}
	return E_FAIL;
}
HRESULT XMFAVSourceReaderRep::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimeStamp, CComPtr<IMFSample> pMFSample)
{
#ifdef DEBUG_TIMMING

	_int64 end;
	_int64 start = 0;
	int count = 0;
	if (dwStreamIndex == 0)
	{
		start = m_startStream0;
		m_count0++;
		count = m_count0;
	}
	else
	{
		start = m_startStream1;
		m_count1++;
		count = m_count1;
	}
	QueryPerformanceCounter((LARGE_INTEGER *) &end);

	if (pMFSample) // log only real samples
	{
		TCHAR timerMessage[512];
		LONGLONG sampleDur = 0;
		pMFSample->GetSampleDuration(&sampleDur);
		_stprintf_s(timerMessage, 512, _T("OnReadSample stream %d took %f ms for sample %d that has a duration of %I64d ms\n"), dwStreamIndex, ((end - start) * 1000.0) / m_freq, count, sampleDur / 10000);
		OutputDebugString(timerMessage);
	}

#endif

	if (FAILED(hrStatus))
	{
		WCHAR mess[1024];
		_com_error err(hrStatus);
		LPCTSTR errMsg = err.ErrorMessage();
		swprintf_s(mess, 1024, L"OnReadSample FAILED - hrStatus(0x%x) err(%s)\n", hrStatus, errMsg);
		OutputDebugStringW(mess);
		return hrStatus;
	}

	HRESULT hr = S_OK;

	if (pMFSample)
	{
		//DWORD totalLength = 0;
		//hr = pMFSample->GetTotalLength(&totalLength);
		//LONGLONG sampleDur = 0;
		//hr = pMFSample->GetSampleDuration(&sampleDur);
		//WCHAR name[1024];
		//swprintf_s(name, 1024, L"stream(%d) numberOfBytes(%d) sampleDur(%I64d) dwStreamFlags(0x%x)", dwStreamIndex, totalLength, sampleDur, dwStreamFlags);
		//DumpAttr(pMFSample, L"OnReadSample", name);
		if (m_bFirstSample)
		{
			m_llBaseTime = llTimeStamp;
			m_bFirstSample = false;
		}

		// re base the time stamp
		llTimeStamp -= m_llBaseTime;

		hr = pMFSample->SetSampleTime(llTimeStamp);

		if (SUCCEEDED_Xb(hr))
		{
			if (m_pXMFSinkWriter)
			{
				bool bStopRequested;
				hr = m_pXMFSinkWriter->WriteSample(dwStreamIndex, pMFSample, &bStopRequested);
				if (bStopRequested)
				{
					m_bCapturePumpIsRunning = false;
					OutputDebugStringW(L"m_pVASourceReader->Flush\n");
					return m_pVASourceReader->Flush((DWORD)MF_SOURCE_READER_ALL_STREAMS);
				}
			}
			else
			{
				OutputDebugStringW(L"NO m_pXMFSinkWriter!!\n");
			}
		}

		if (SUCCEEDED_Xb(hr))
		{
#ifdef DEBUG_TIMMING

			if (dwStreamIndex == 0)
			{
				QueryPerformanceCounter((LARGE_INTEGER *) &m_startStream0);
			}
			else
			{
				QueryPerformanceCounter((LARGE_INTEGER *) &m_startStream1);
			}
#endif

			hr = m_pVASourceReader->ReadSample((DWORD) MF_SOURCE_READER_ANY_STREAM, 0, NULL, NULL, NULL, NULL);
		}
	}
	else
	{

#ifdef DEBUG_TIMMING

		WCHAR mess[1024];
		swprintf_s(mess, 1024, L"OnReadSample NO SAMPLE - stream(%d) dwStreamFlags(0x%x)\n", dwStreamIndex, dwStreamFlags);
		OutputDebugStringW(mess);
		if (dwStreamIndex == 0)
		{
			QueryPerformanceCounter((LARGE_INTEGER *) &m_startStream0);
		}
		else
		{
			QueryPerformanceCounter((LARGE_INTEGER *) &m_startStream1);
		}

#endif

		hr = m_pVASourceReader->ReadSample((DWORD) MF_SOURCE_READER_ANY_STREAM, 0, NULL, NULL, NULL, NULL);
	}

	//if (FAILED(hr))
	//{
	//	NotifyError(hr);
	//}

	return hr;
}

bool XMFAVSourceReader::Capturing()
{
	if (m_pRep)
	{
		return m_pRep->Capturing();
	}
	return false;
}
bool XMFAVSourceReaderRep::Capturing()
{
	return m_bCapturePumpIsRunning;
}

HRESULT XMFAVSourceReader::Start()
{
	if (m_pRep)
	{
		return m_pRep->Start();
	}
	return E_FAIL;
}
HRESULT XMFAVSourceReaderRep::Start()
{
	m_bFirstSample = true;
	m_llBaseTime = 0;

#ifdef DEBUG_TIMMING

	QueryPerformanceFrequency((LARGE_INTEGER *) &m_freq);
	QueryPerformanceCounter((LARGE_INTEGER *) &m_startStream0);
	QueryPerformanceCounter((LARGE_INTEGER *) &m_startStream1);
	m_count0 = 0;
	m_count1 = 0;

#endif

	m_bCapturePumpIsRunning = true;

	return m_pVASourceReader->ReadSample((DWORD) MF_SOURCE_READER_ANY_STREAM, 0, NULL, NULL, NULL, NULL);
}

HRESULT XMFAVSourceReaderRep::ConfigureVASourceReaderVideo()
{
	// The list of acceptable types.
	GUID subtypes [] = { MEDIASUBTYPE_HDYC, MFVideoFormat_v210, MFVideoFormat_NV12, MFVideoFormat_YUY2, MFVideoFormat_UYVY, MFVideoFormat_RGB32, MFVideoFormat_RGB24, MFVideoFormat_IYUV };

	HRESULT hr = S_OK;
	BOOL    bUseNativeType = FALSE;

	GUID subtype = { 0 };

	// TODO: Write a comprehensive native format selector
	CComPtr<IMFMediaType> pVideoReaderInputMediaTypeNative = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		//DWORD formatWeWant = 12;	// BM is 12 for 720p 5994 HDYC
		//DWORD formatWeWant = 18;	// BM is 18 for 1080i 5994 HDYC
		//DWORD formatWeWant = 0;	// WEBCAM is 0 for 640 30fps YUY2
		DWORD formatWeWant = 64;	// MXL is 64 for 720p 5994 YUY2
		hr = m_pVASourceReader->GetNativeMediaType((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, formatWeWant, &pVideoReaderInputMediaTypeNative);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = pVideoReaderInputMediaTypeNative->GetGUID(MF_MT_SUBTYPE, &subtype);
	}

	if (SUCCEEDED_Xb(hr))
	{
		for (UINT32 i = 0; i < ARRAYSIZE(subtypes); i++)
		{
			if (subtype == subtypes[i])
			{
				hr = m_pVASourceReader->SetCurrentMediaType((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pVideoReaderInputMediaTypeNative);
				bUseNativeType = TRUE;
				break;
			}
		}

		if (!bUseNativeType)
		{
			// None of the native types worked. The camera might offer 
			// output a compressed type such as MJPEG or DV.
			// Try adding a decoder.
			for (UINT32 i = 0; i < ARRAYSIZE(subtypes); i++)
			{
				hr = pVideoReaderInputMediaTypeNative->SetGUID(MF_MT_SUBTYPE, subtypes[i]);

				if (SUCCEEDED_Xb(hr))
				{
					hr = m_pVASourceReader->SetCurrentMediaType((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pVideoReaderInputMediaTypeNative);
				}
				else
				{
					break;
				}

				if (SUCCEEDED_Xb(hr))
				{
					break;
				}
			}
		}
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pVASourceReader->SetStreamSelection((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
	}

	return hr;
}

HRESULT XMFAVSourceReaderRep::ConfigureVASourceReaderAudio()
{
	HRESULT hr = S_OK;

	// TODO: Write a comprehensive native format selector
	CComPtr<IMFMediaType> pAudioReaderInputMediaTypeNative = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		DWORD formatWeWant = 0;		// 2 channels 48k 16 bit PCM
		hr = m_pVASourceReader->GetNativeMediaType((DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, formatWeWant, &pAudioReaderInputMediaTypeNative);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pVASourceReader->SetCurrentMediaType((DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pAudioReaderInputMediaTypeNative);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pVASourceReader->SetStreamSelection((DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
	}

	return hr;
}

HRESULT XMFAVSourceReader::GetVideoInputMediaType(CComPtr<IMFMediaType>& pVideoInputMediaType)
{
	if (m_pRep)
	{
		return m_pRep->GetVideoInputMediaType(pVideoInputMediaType);
	}
	return E_FAIL;
}
HRESULT XMFAVSourceReaderRep::GetVideoInputMediaType(CComPtr<IMFMediaType>& pVideoInputMediaType)
{
	if (m_pVASourceReader)
	{
		return m_pVASourceReader->GetCurrentMediaType((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pVideoInputMediaType);
	}
	return E_FAIL;
}

HRESULT XMFAVSourceReader::GetAudioInputMediaType(CComPtr<IMFMediaType>& pAudioInputMediaType)
{
	if (m_pRep)
	{
		return m_pRep->GetAudioInputMediaType(pAudioInputMediaType);
	}
	return E_FAIL;
}
HRESULT XMFAVSourceReaderRep::GetAudioInputMediaType(CComPtr<IMFMediaType>& pAudioInputMediaType)
{
	if (m_pVASourceReader)
	{
		return m_pVASourceReader->GetCurrentMediaType((DWORD) MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pAudioInputMediaType);
	}
	return E_FAIL;
}

HRESULT XMFAVSourceReader::GetPresentationDescriptor(CComPtr<IMFPresentationDescriptor>& pVideoInputPresentationDescriptor)
{
	if (m_pRep)
	{
		return m_pRep->GetPresentationDescriptor(pVideoInputPresentationDescriptor);
	}
	return E_FAIL;
}
HRESULT XMFAVSourceReaderRep::GetPresentationDescriptor(CComPtr<IMFPresentationDescriptor>& pVideoInputPresentationDescriptor)
{
	if (m_pVASource)
	{
		return m_pVASource->CreatePresentationDescriptor(&pVideoInputPresentationDescriptor);
	}
	return E_FAIL;
}

HRESULT XMFAVSourceReader::GetMFMediaSource(CComPtr<IMFMediaSource>& pMFMediaSource)
{
	if (m_pRep)
	{
		return m_pRep->GetMFMediaSource(pMFMediaSource);
	}
	return E_FAIL;
}
HRESULT XMFAVSourceReaderRep::GetMFMediaSource(CComPtr<IMFMediaSource>& pMFMediaSource)
{
	if (m_pVASourceReader)
	{
		pMFMediaSource = m_pVASource;
		return S_OK;
	}
	return E_FAIL;
}