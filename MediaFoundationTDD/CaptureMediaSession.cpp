#include "CaptureMediaSession.h"
#include "MFUtils.h"
#include "Topology.h"
#include "VideoDisplayControl.h"
#include "VideoDevices.h"
#include "AudioDevices.h"
#include "MediaSource.h"

#include <mfapi.h>
#include <atlbase.h>
#include <mfidl.h>

class CaptureMediaSessionRep : public IMFAsyncCallback, public MFUtils
{
public:
	CaptureMediaSessionRep(std::wstring videoDeviceName, std::wstring audioDeviceName, std::wstring captureFilePath);
	~CaptureMediaSessionRep();

	HRESULT								GetLastHRESULT();

	void								InitCaptureAndPassthrough(HWND videoWindow, std::wstring captureFileName);
	void								Start();
	void								Stop();

	// IMFAsyncCallback
	STDMETHODIMP						GetParameters(DWORD* /*pdwFlags*/, DWORD* /*pdwQueue*/) { return E_NOTIMPL; }
	STDMETHODIMP						Invoke(IMFAsyncResult* pAsyncResult);

	// IUnknown
	STDMETHODIMP						QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG)				AddRef();
	STDMETHODIMP_(ULONG)				Release();

private:
	void								CreateCaptureSourceAndRenderers(HWND videoWindow);

	void								ProcessMediaEvent(CComPtr<IMFMediaEvent> mediaEvent);

	void								DumpTopologyFailed(CComPtr<IMFMediaEvent> mediaEvent);

	std::wstring								mVideoDeviceName = L"";
	std::wstring								mAudioDeviceName = L"";
	std::wstring								mCaptureFilePath = L"";

	std::shared_ptr<MediaSource>				mCaptureSource = nullptr;
	CComPtr<IMFActivate>						mVideoRenderer = nullptr;
	CComPtr<IMFActivate>						mAudioRenderer = nullptr;
	std::shared_ptr<VideoDisplayControl>		mVideoDisplayControl = nullptr;
	std::shared_ptr<OnTopologyReadyCallback>	mOnTopologyReadyCallback = nullptr;

	std::unique_ptr<Topology>					mTopology = nullptr;
		
	CComPtr<IMFMediaSession>					mMediaSession = nullptr;

	CComAutoCriticalSection						mCritSec;
	volatile long								mRefCount					= 1;

	HANDLE										mStartedEvent				= INVALID_HANDLE_VALUE;
	HANDLE										mStoppedEvent				= INVALID_HANDLE_VALUE;
};

CaptureMediaSession::CaptureMediaSession(std::wstring videoDeviceName, std::wstring audioDeviceName, std::wstring captureFilePath)
{
	m_pRep = std::unique_ptr<CaptureMediaSessionRep>(new CaptureMediaSessionRep(videoDeviceName, audioDeviceName, captureFilePath));
}
CaptureMediaSessionRep::CaptureMediaSessionRep(std::wstring videoDeviceName, std::wstring audioDeviceName, std::wstring captureFilePath):
	mVideoDeviceName(videoDeviceName),
	mAudioDeviceName(audioDeviceName),
	mCaptureFilePath(captureFilePath)
{
	OnERR_return(MFStartup(MF_VERSION));
	OnERR_return(MFCreateMediaSession(nullptr, &mMediaSession));

	mStoppedEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mStoppedEvent == INVALID_HANDLE_VALUE)
	{
		OnERR_return(HRESULT_FROM_WIN32(GetLastError()));
	}
	mStartedEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mStartedEvent == INVALID_HANDLE_VALUE)
	{
		OnERR_return(HRESULT_FROM_WIN32(GetLastError()));
	}
	mTopology = std::make_unique<Topology>();
}
CaptureMediaSession::~CaptureMediaSession()
{
}
CaptureMediaSessionRep::~CaptureMediaSessionRep()
{
	OnERR_return(MFShutdown());
}

HRESULT CaptureMediaSession::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT CaptureMediaSessionRep::GetLastHRESULT()
{
	return MFUtils::GetLastHRESULT();
}

HRESULT CaptureMediaSessionRep::Invoke(IMFAsyncResult* pAsyncResult)
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
	OnERR_return_HR(mMediaSession->BeginGetEvent(this, nullptr));
	return S_OK;
}

HRESULT CaptureMediaSessionRep::QueryInterface(REFIID riid, void** ppv)
{
	HRESULT hr = S_OK;
	if (ppv == nullptr)
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
		*ppv = nullptr;
		hr = E_NOINTERFACE;
	}
	if (SUCCEEDED(hr))
	{
		AddRef();
	}
	return hr;
}
ULONG CaptureMediaSessionRep::AddRef()
{
	return InterlockedIncrement(&mRefCount);
}
ULONG CaptureMediaSessionRep::Release()
{
	ULONG uCount = InterlockedDecrement(&mRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}

void CaptureMediaSessionRep::DumpTopologyFailed(CComPtr<IMFMediaEvent> mediaEvent)
{
	auto topology = std::make_unique<Topology>(mediaEvent);
	topology->DumpTopology(nullptr);
}

void CaptureMediaSessionRep::ProcessMediaEvent(CComPtr<IMFMediaEvent> mediaEvent)
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
			DumpTopologyFailed(mediaEvent);
		}
		SetLastHR_Fail();
		return;
	}
	switch (eventType)
	{
	case MESessionTopologyStatus:
		OnERR_return(mediaEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus));
		if (TopoStatus == MF_TOPOSTATUS_READY && mOnTopologyReadyCallback)
		{
			mOnTopologyReadyCallback->OnTopologyReady(mMediaSession);
		}
		break;
	case MESessionStarted:
		SetEvent(mStartedEvent);
		break;
	case MF_TOPOSTATUS_STARTED_SOURCE:
		OutputDebugStringW(L"MF_TOPOSTATUS_STARTED_SOURCE\n");
		break;
	case MESessionStopped:
		OnERR_return(mMediaSession->Close());
		break;
	case MESessionClosed:
		SetEvent(mStoppedEvent);
		break;
	case MESessionEnded:
		OutputDebugStringW(L"MESessionEnded\n");
		//OnERR_return(mMediaSession->Stop());
		break;
	default:
		OutputDebugStringW(L"UKNOWN IMFMediaEvent!\n");
		break;
	}
}

void CaptureMediaSession::Start()
{
	m_pRep->Start();
}
void CaptureMediaSessionRep::Start()
{
	OnERR_return(mMediaSession->BeginGetEvent((IMFAsyncCallback*)this, nullptr));
	PROPVARIANT varStart;
	PropVariantInit(&varStart);
	varStart.vt = VT_EMPTY;
	OnERR_return(mMediaSession->Start(&GUID_NULL, &varStart));
	DWORD res = WaitForSingleObject(mStartedEvent, INFINITE);
}

void CaptureMediaSession::Stop()
{
	m_pRep->Stop();
}
void CaptureMediaSessionRep::Stop()
{
	OnERR_return(mMediaSession->Stop());
	DWORD res = WaitForSingleObject(mStoppedEvent, INFINITE);
}

void CaptureMediaSessionRep::CreateCaptureSourceAndRenderers(HWND videoWindow)
{
	VideoDevices videoDevices(videoWindow);
	mVideoDisplayControl = std::make_shared<VideoDisplayControl>(videoWindow);
	mVideoRenderer = videoDevices.GetVideoRenderer();

	AudioDevices audioDevices;
	mAudioRenderer = audioDevices.GetAudioRenderer();

	mCaptureSource = std::make_shared<MediaSource>(videoDevices.GetCaptureVideoDevice(mVideoDeviceName), audioDevices.GetCaptureAudioDevice(mAudioDeviceName));
}

void CaptureMediaSession::InitCaptureAndPassthrough(HWND videoWindow, std::wstring captureFileName)
{
	m_pRep->InitCaptureAndPassthrough(videoWindow, captureFileName);
}
void CaptureMediaSessionRep::InitCaptureAndPassthrough(HWND videoWindow, std::wstring captureFileName)
{
	CreateCaptureSourceAndRenderers(videoWindow);
	std::wstring fileToWrite = mCaptureFilePath + captureFileName;
	mTopology->CreateTopology(mCaptureSource, fileToWrite, mVideoRenderer, mAudioRenderer, mMediaSession);
}