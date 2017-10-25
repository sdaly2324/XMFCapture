#include "MediaSession.h"
#include "MFUtils.h"

#include <mfapi.h>
#include <atlbase.h>
#include <mfidl.h>

class MediaSessionRep : public IMFAsyncCallback, public MFUtils
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
	void								ProcessMediaEvent(CComPtr<IMFMediaEvent>& mediaEvent);

	CComAutoCriticalSection						mCritSec;
	CComPtr<IMFMediaSession>					mMediaSession				= NULL;
	volatile long								mRefCount					= 1;
	HANDLE										mStartedEvent				= INVALID_HANDLE_VALUE;
	HANDLE										mStoppedEvent				= INVALID_HANDLE_VALUE;
	std::shared_ptr<OnTopologyReadyCallback>	mOnTopologyReadyCallback	= NULL;
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

	mStoppedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (mStoppedEvent == INVALID_HANDLE_VALUE)
	{
		OnERR_return(HRESULT_FROM_WIN32(GetLastError()));
	}
	mStartedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (mStartedEvent == INVALID_HANDLE_VALUE)
	{
		OnERR_return(HRESULT_FROM_WIN32(GetLastError()));
	}
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
	return MFUtils::GetLastHRESULT();
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
	CComCritSecLock<CComAutoCriticalSection> lock(mCritSec);
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
	return InterlockedIncrement(&mRefCount);
}
ULONG MediaSessionRep::Release()
{
	ULONG uCount = InterlockedDecrement(&mRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}

void MediaSessionRep::ProcessMediaEvent(CComPtr<IMFMediaEvent>& mediaEvent)
{
	HRESULT hrStatus = S_OK;            // Event status
	HRESULT hr = S_OK;
	UINT32 TopoStatus = MF_TOPOSTATUS_INVALID;

	MediaEventType eventType;
	OnERR_return(mediaEvent->GetType(&eventType));
	OnERR_return(mediaEvent->GetStatus(&hrStatus));
	if (IsHRError(hrStatus))
	{
		if (eventType == MESessionTopologySet)
		{
			PROPVARIANT var;
			PropVariantInit(&var);
			hr = mediaEvent->GetValue(&var);
			CComPtr<IMFTopology> topology = NULL;
			hr = var.punkVal->QueryInterface(__uuidof(IMFTopology), (void**)&topology);

			//CComPtr<IMFCollection> sourceNodeCollection = NULL;
			//hr = topology->GetSourceNodeCollection(&sourceNodeCollection);
			//DWORD sourceElements = 0;
			//hr = sourceNodeCollection->GetElementCount(&sourceElements);
			//CComPtr<IMFCollection> outputNodeCollection = NULL;
			//hr = topology->GetOutputNodeCollection(&outputNodeCollection);
			//DWORD outputElements = 0;
			//hr = outputNodeCollection->GetElementCount(&outputElements);

			WORD nodeCount = 0;
			hr = topology->GetNodeCount(&nodeCount);
			for (int i = 0; i < nodeCount; i++)
			{
				CComPtr<IMFTopologyNode> node;
				hr = topology->GetNode(i, &node);
				DumpAttr(node, L"IMFTopologyNode" , std::to_wstring(i));
				DWORD inputs = 0;
				hr = node->GetInputCount(&inputs);
				for (unsigned int inIndex = 0; inIndex < inputs; inIndex++)
				{
					CComPtr<IMFMediaType> inputPrefType = NULL;
					if (!IsHRError(node->GetInputPrefType(inIndex, &inputPrefType)))
					{
						DumpAttr(inputPrefType, L"node " + std::to_wstring(i), L"input " + std::to_wstring(inIndex));
					}
				}
				DWORD outputs = 0;
				hr = node->GetOutputCount(&outputs);
				for (unsigned int outIndex = 0; outIndex < outputs; outIndex++)
				{
					CComPtr<IMFMediaType> outputPrefType = NULL;
					if (!IsHRError(node->GetOutputPrefType(outIndex, &outputPrefType)))
					{
						DumpAttr(outputPrefType, L"node " + std::to_wstring(i), L"output " + std::to_wstring(outIndex));
					}
				}
				OutputDebugStringW(L"\n");
			}
		}
		SetLastHR_Fail();
		return;
	}
	if (eventType == MESessionTopologyStatus)
	{
		OnERR_return(mediaEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus));
		if (TopoStatus == MF_TOPOSTATUS_READY && mOnTopologyReadyCallback)
		{
			mOnTopologyReadyCallback->OnTopologyReady(mMediaSession);
		}
	}
	else if (eventType == MESessionStopped)
	{
		SetEvent(mStoppedEvent);
	}
	else if (eventType == MESessionStarted)
	{
		SetEvent(mStartedEvent);
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
	DWORD res = WaitForSingleObject(mStartedEvent, INFINITE);
}

void MediaSession::Stop()
{
	m_pRep->Stop();
}
void MediaSessionRep::Stop()
{
	OnERR_return(mMediaSession->Stop());
	DWORD res = WaitForSingleObject(mStoppedEvent, INFINITE);
}