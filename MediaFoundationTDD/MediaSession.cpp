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
	PrintIfErrAndSave(MFStartup(MF_VERSION));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(MFCreateMediaSession(NULL, &mMediaSession));
	}
}
MediaSession::~MediaSession()
{
}
MediaSessionRep::~MediaSessionRep()
{
	PrintIfErrAndSave(MFShutdown());
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
	PrintIfErrAndSave(mMediaSession->EndGetEvent(pAsyncResult, &pEvent));
	if (!LastHR_OK())
	{
		return GetLastHRESULT();
	}
	ProcessMediaEvent(pEvent);
	if (!LastHR_OK())
	{
		return GetLastHRESULT();
	}
	PrintIfErrAndSave(mMediaSession->BeginGetEvent(this, NULL));
	if (!LastHR_OK())
	{
		return GetLastHRESULT();
	}
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
	PrintIfErrAndSave(pMediaEvent->GetType(&eventType));
	if (!LastHR_OK())
	{
		return;
	}
	PrintIfErrAndSave(pMediaEvent->GetStatus(&hrStatus));
	if (!LastHR_OK())
	{
		return;
	}
	if (FAILED(hrStatus))
	{
		PrintIfErrAndSave(hrStatus);
		SetLastHR_Fail();
		return;
	}
	if (eventType == MESessionTopologyStatus)
	{
		PrintIfErrAndSave(pMediaEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus));
		if (!LastHR_OK())
		{
			return;
		}
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
	PrintIfErrAndSave(mMediaSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));
	if (LastHR_OK())
	{
		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		varStart.vt = VT_EMPTY;
		PrintIfErrAndSave(mMediaSession->Start(&GUID_NULL, &varStart));
	}
}

void MediaSession::Stop()
{
	m_pRep->Stop();
}
void MediaSessionRep::Stop()
{
	PrintIfErrAndSave(mMediaSession->Stop());
}