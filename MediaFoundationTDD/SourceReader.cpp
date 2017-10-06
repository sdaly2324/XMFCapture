#include "SourceReader.h"
#include "IMFWrapper.h"
#include "AttributesFactory.h"

#include <mfidl.h>
#include <mfreadwrite.h>
#include <Shlwapi.h>

class SourceReaderRep : public IMFSourceReaderCallback, public IMFWrapper
{
public:
	SourceReaderRep(CComPtr<IMFMediaSource> mediaSource);
	~SourceReaderRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFSourceReader>			GetSourceReader();
	CComPtr<IMFMediaSource>				GetMediaSource();
	CComPtr<IMFPresentationDescriptor>	GetPresentationDescriptor();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFSourceReaderCallback methods
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD streamFlags, LONGLONG timestamp, IMFSample* sample);
	STDMETHODIMP OnEvent(DWORD streamIndex, IMFMediaEvent* pEvent);
	STDMETHODIMP OnFlush(DWORD dwStreamIndex);

private:
	CComPtr<IMFMediaSource>		mMediaSource = NULL;
	CComPtr<IMFSourceReader>	mSourceReader = NULL;
	long mRefCount = 0;
};
SourceReader::SourceReader(CComPtr<IMFMediaSource> mediaSource)
{
	m_pRep = new SourceReaderRep(mediaSource);
}
SourceReaderRep::SourceReaderRep(CComPtr<IMFMediaSource> mediaSource)
{
	mMediaSource = mediaSource;

	AttributesFactory* attributesFactory = new AttributesFactory();
	CComPtr<IMFAttributes> sourceReaderAsycCallbackAttributes = attributesFactory->CreateSourceReaderAsycCallbackAttributes(this);

	PrintIfErrAndSave(MFCreateSourceReaderFromMediaSource(mediaSource, sourceReaderAsycCallbackAttributes, &mSourceReader));
}
SourceReader::~SourceReader()
{
	delete m_pRep;
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
	return IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFSourceReader> SourceReader::GetSourceReader()
{
	return m_pRep->GetSourceReader();
}
CComPtr<IMFSourceReader> SourceReaderRep::GetSourceReader()
{
	return mSourceReader;
}

CComPtr<IMFMediaSource> SourceReader::GetMediaSource()
{
	return m_pRep->GetMediaSource();
}
CComPtr<IMFMediaSource> SourceReaderRep::GetMediaSource()
{
	return mMediaSource;
}

CComPtr<IMFPresentationDescriptor> SourceReader::GetPresentationDescriptor()
{
	return m_pRep->GetPresentationDescriptor();
}
CComPtr<IMFPresentationDescriptor> SourceReaderRep::GetPresentationDescriptor()
{
	CComPtr<IMFPresentationDescriptor> retVal = NULL;
	PrintIfErrAndSave(mMediaSource->CreatePresentationDescriptor(&retVal));
	if (!LastHR_OK() || !retVal)
	{
		return NULL;
	}
	return retVal;
}

HRESULT SourceReaderRep::OnEvent(DWORD streamIndex, IMFMediaEvent* pEvent)
{
	MediaEventType mediaEventType = MEUnknown;
	PrintIfErrAndSave(pEvent->GetType(&mediaEventType));
	if (LastHR_OK())
	{
		if (mediaEventType != MESourceCharacteristicsChanged)
		{
			WCHAR mess[1024];
			swprintf_s(mess, 1024, L"SourceReaderCallbackRep::OnEvent stream(%d) type(%d)\n", streamIndex, mediaEventType);
			OutputDebugStringW(mess);
		}
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
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}