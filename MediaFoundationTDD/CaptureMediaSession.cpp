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
#include <Mferror.h>

class CaptureMediaSessionRep : public IMFAsyncCallback, public MFUtils
{
public:
	CaptureMediaSessionRep(std::wstring videoDeviceName, std::wstring audioDeviceName, std::wstring captureFilePath);
	~CaptureMediaSessionRep();

	HRESULT								GetLastHRESULT();

	void								InitCaptureAndPassthrough(HWND videoWindow, std::wstring captureFileName);
	void								InitPassthrough(HWND videoWindow);
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
	if (mMediaSession)
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
		HRESULT hr = mMediaSession->BeginGetEvent(this, nullptr);
		if (!SUCCEEDED(hr) && hr != MF_E_SHUTDOWN)
		{
			return hr;
		}
		if (hr == MF_E_SHUTDOWN)
		{
			OutputDebugStringW(L"Invoke MF_E_SHUTDOWN\n");
		}
	}
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
	case MESessionTopologySet:
		OutputDebugStringW(L"MESessionTopologySet\n");
		break;
	case MESessionCapabilitiesChanged:
		OutputDebugStringW(L"MESessionCapabilitiesChanged\n");
		break;
	case MESessionNotifyPresentationTime:
		OutputDebugStringW(L"MESessionNotifyPresentationTime\n");
		break;
	case MESessionStarted:
		OutputDebugStringW(L"MESessionStarted\n");
		SetEvent(mStartedEvent);
		break;
	case MESessionStopped:
		OutputDebugStringW(L"MESessionStopped\n");
		OnERR_return(mMediaSession->Close());
		break;
	case MESessionClosed:
		OutputDebugStringW(L"MESessionClosed\n");
		OnERR_return(mMediaSession->Shutdown());
		SetEvent(mStoppedEvent);
		break;
	case MESessionEnded:
		OutputDebugStringW(L"MESessionEnded\n");
		break;
	case MESessionStreamSinkFormatChanged:
		OutputDebugStringW(L"MESessionStreamSinkFormatChanged\n");
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

	mMediaSession.Release();
	OnERR_return(MFCreateMediaSession(nullptr, &mMediaSession));
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
	mTopology.reset(nullptr);
	mTopology = std::make_unique<Topology>();
	CreateCaptureSourceAndRenderers(videoWindow);
	std::wstring fileToWrite = mCaptureFilePath + captureFileName;
	mTopology->CreateTopology(mCaptureSource, fileToWrite, mVideoRenderer, mAudioRenderer, mMediaSession);
}

void CaptureMediaSession::InitPassthrough(HWND videoWindow)
{
	m_pRep->InitPassthrough(videoWindow);
}
void CaptureMediaSessionRep::InitPassthrough(HWND videoWindow)
{
	mTopology.reset(nullptr);
	mTopology = std::make_unique<Topology>();
	CreateCaptureSourceAndRenderers(videoWindow);
	mTopology->CreateTopology(mCaptureSource, L"", mVideoRenderer, mAudioRenderer, mMediaSession);
}