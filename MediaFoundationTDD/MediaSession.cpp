#include "MediaSession.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <atlbase.h>
#include <mfidl.h>

class MediaSessionRep : public IMFAsyncCallback, public IMFWrapper
{
public:
	MediaSessionRep(std::shared_ptr<OnTopologyReadyCallback> onTopologyReadyCallback);
	~MediaSessionRep();

	HRESULT								GetLastHRESULT();

	void								Start();
	void								Stop();

	// IMFAsyncCallback
	STDMETHODIMP						GetParameters(DWORD* /*pdwFlags*/, DWORD* /*pdwQueue*/) { return E_NOTIMPL; }
	STDMETHODIMP						Invoke(IMFAsyncResult* pAsyncResult);

	// IUnknown
	STDMETHODIMP						QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG)				AddRef();
	STDMETHODIMP_(ULONG)				Release();

	CComPtr<IMFMediaSession>			GetMediaSession();

private:
	void								ProcessMediaEvent(CComPtr<IMFMediaEvent>& pMediaEvent);

	CComPtr<IMFMediaSession>			mMediaSession = NULL;
	CComAutoCriticalSection				m_critSec;
	volatile long						m_nRefCount = 1;

	std::shared_ptr<OnTopologyReadyCallback>			mOnTopologyReadyCallback = NULL;
};

MediaSession::MediaSession(std::shared_ptr<OnTopologyReadyCallback> onTopologyReadyCallback)
{
	m_pRep = std::unique_ptr<MediaSessionRep>(new MediaSessionRep(onTopologyReadyCallback));
}
MediaSessionRep::MediaSessionRep(std::shared_ptr<OnTopologyReadyCallback> onTopologyReadyCallback):
	mOnTopologyReadyCallback(onTopologyReadyCallback)
{
	OnERR_return(MFStartup(MF_VERSION));
	OnERR_return(MFCreateMediaSession(NULL, &mMediaSession));
}
MediaSession::~MediaSession()
{
}
MediaSessionRep::~MediaSessionRep()
{
	OnERR_return(MFShutdown());
}

HRESULT MediaSession::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT MediaSessionRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFMediaSession> MediaSession::GetMediaSession()
{
	return m_pRep->GetMediaSession();
}
CComPtr<IMFMediaSession> MediaSessionRep::GetMediaSession()
{
	return mMediaSession;
}

HRESULT MediaSessionRep::Invoke(IMFAsyncResult* pAsyncResult)
{
	CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);
	if (!LastHR_OK())
	{
		return GetLastHRESULT();
	}
	CComPtr<IMFMediaEvent> pEvent;
	OnERR_return_HR(mMediaSession->EndGetEvent(pAsyncResult, &pEvent));
	ProcessMediaEvent(pEvent);
	if (!LastHR_OK())
	{
		return GetLastHRESULT();
	}
	OnERR_return_HR(mMediaSession->BeginGetEvent(this, NULL));
	return S_OK;
}

HRESULT MediaSessionRep::QueryInterface(REFIID riid, void** ppv)
{
	HRESULT hr = S_OK;
	if (ppv == NULL)
	{
		return E_POINTER;
	}
	if (riid == __uuidof(IMFAsyncCallback))
	{
		*ppv = static_cast<IMFAsyncCallback*>(this);
	}
	else if (riid == __uuidof(IUnknown))
	{
		*ppv = static_cast<IUnknown*>(this);
	}
	else
	{
		*ppv = NULL;
		hr = E_NOINTERFACE;
	}
	if (SUCCEEDED(hr))
	{
		AddRef();
	}
	return hr;
}
ULONG MediaSessionRep::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}
ULONG MediaSessionRep::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}

void MediaSessionRep::ProcessMediaEvent(CComPtr<IMFMediaEvent>& pMediaEvent)
{
	HRESULT hrStatus = S_OK;            // Event status
	HRESULT hr = S_OK;
	UINT32 TopoStatus = MF_TOPOSTATUS_INVALID;

	MediaEventType eventType;
	OnERR_return(pMediaEvent->GetType(&eventType));
	OnERR_return(pMediaEvent->GetStatus(&hrStatus));
	if (IsHRError(hrStatus))
	{
		SetLastHR_Fail();
		return;
	}
	if (eventType == MESessionTopologyStatus)
	{
		OnERR_return(pMediaEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus));
		if (TopoStatus == MF_TOPOSTATUS_READY && mOnTopologyReadyCallback)
		{
			mOnTopologyReadyCallback->OnTopologyReady(mMediaSession);
		}
	}
}

void MediaSession::Start()
{
	m_pRep->Start();
}
void MediaSessionRep::Start()
{
	OnERR_return(mMediaSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));
	PROPVARIANT varStart;
	PropVariantInit(&varStart);
	varStart.vt = VT_EMPTY;
	OnERR_return(mMediaSession->Start(&GUID_NULL, &varStart));
}

void MediaSession::Stop()
{
	m_pRep->Stop();
}
void MediaSessionRep::Stop()
{
	OnERR_return(mMediaSession->Stop());
}