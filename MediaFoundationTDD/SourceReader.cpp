#include "SourceReader.h"
#include "MFUtils.h"
#include "AttributesFactory.h"

#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <Shlwapi.h>

class SourceReaderRep : public IMFSourceReaderCallback, public MFUtils
{
public:
	SourceReaderRep(CComPtr<IMFMediaSource> mediaSource);
	~SourceReaderRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFSourceReader>			GetSourceReader();
	CComPtr<IMFMediaType>				GetVideoMediaType();
	CComPtr<IMFMediaType>				GetAudioMediaType();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFSourceReaderCallback methods
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD streamFlags, LONGLONG timestamp, IMFSample* sample);
	STDMETHODIMP OnEvent(DWORD streamIndex, IMFMediaEvent* pEvent);
	STDMETHODIMP OnFlush(DWORD dwStreamIndex);

private:
	CComPtr<IMFSourceReader>	mSourceReader = NULL;
	long mRefCount = 0;
};
SourceReader::SourceReader(CComPtr<IMFMediaSource> mediaSource)
{
	m_pRep = std::unique_ptr<SourceReaderRep>(new SourceReaderRep(mediaSource));
}
SourceReaderRep::SourceReaderRep(CComPtr<IMFMediaSource> mediaSource)
{
	AttributesFactory* attributesFactory = new AttributesFactory();
	CComPtr<IMFAttributes> sourceReaderAsycCallbackAttributes = attributesFactory->CreateSReaderCbAttrs(this);

	OnERR_return(MFCreateSourceReaderFromMediaSource(mediaSource, sourceReaderAsycCallbackAttributes, &mSourceReader));
}
SourceReader::~SourceReader()
{
}
SourceReaderRep::~SourceReaderRep()
{
}

HRESULT SourceReader::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT SourceReaderRep::GetLastHRESULT()
{
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFSourceReader> SourceReader::GetSourceReader()
{
	return m_pRep->GetSourceReader();
}
CComPtr<IMFSourceReader> SourceReaderRep::GetSourceReader()
{
	return mSourceReader;
}

HRESULT SourceReaderRep::OnEvent(DWORD streamIndex, IMFMediaEvent* pEvent)
{
	MediaEventType mediaEventType = MEUnknown;
	OnERR_return_HR(pEvent->GetType(&mediaEventType));
	if (mediaEventType != MESourceCharacteristicsChanged)
	{
		WCHAR mess[1024];
		swprintf_s(mess, 1024, L"SourceReaderCallbackRep::OnEvent stream(%d) type(%d)\n", streamIndex, mediaEventType);
		OutputDebugStringW(mess);
	}
	return S_OK;
}

HRESULT SourceReaderRep::OnFlush(DWORD streamIndex)
{
	return E_FAIL;
}

HRESULT SourceReaderRep::OnReadSample(HRESULT hrStatus, DWORD streamIndex, DWORD streamFlags, LONGLONG llTimeStamp, IMFSample* sample)
{
	return E_FAIL;
}

HRESULT SourceReaderRep::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(SourceReaderRep, IMFSourceReaderCallback),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

ULONG SourceReaderRep::AddRef()
{
	return InterlockedIncrement(&mRefCount);
}

ULONG SourceReaderRep::Release()
{
	ULONG uCount = InterlockedDecrement(&mRefCount);
	return uCount;
}

CComPtr<IMFMediaType> SourceReader::GetVideoMediaType()
{
	return m_pRep->GetVideoMediaType();
}
CComPtr<IMFMediaType> SourceReaderRep::GetVideoMediaType()
{
	DWORD formatWeWant = 64;	// MXL is 64 for 720p 5994 YUY2
	CComPtr<IMFMediaType> retval = NULL;
	OnERR_return_NULL(mSourceReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, formatWeWant, &retval));
	return retval;
}

CComPtr<IMFMediaType> SourceReader::GetAudioMediaType()
{
	return m_pRep->GetAudioMediaType();
}
CComPtr<IMFMediaType> SourceReaderRep::GetAudioMediaType()
{
	DWORD formatWeWant = 0;		// 2 channels 48k 16 bit PCM
	CComPtr<IMFMediaType> retval = NULL;
	OnERR_return_NULL(mSourceReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, formatWeWant, &retval));
	return retval;
}