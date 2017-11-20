#include "CaptureMediaSession.h"
#include "MFUtils.h"
#include "Topology.h"
#include "VideoDevices.h"
#include "AudioDevices.h"
#include "MediaSource.h"

#include <mfapi.h>
#include <atlbase.h>
#include <mfidl.h>
#include <Mferror.h>
#include <evr.h>

class CaptureMediaSessionRep : public IMFAsyncCallback, public MFUtils
{
public:
	CaptureMediaSessionRep(std::wstring videoDeviceName, std::wstring audioDeviceName, std::wstring captureFilePath);
	virtual ~CaptureMediaSessionRep();

	HRESULT								GetLastHRESULT();

	void								StartPreview(HWND videoWindow);
	void								StopPreview();
	void								StartCapture(HWND videoWindow, std::wstring captureFileName);
	void								PauseCapture();
	void								ResumeCapture();
	void								StopCaptureImp();

	long long							GetTime();

	// IMFAsyncCallback
	STDMETHODIMP						GetParameters(DWORD* /*pdwFlags*/, DWORD* /*pdwQueue*/) { return E_NOTIMPL; }
	STDMETHODIMP						Invoke(IMFAsyncResult* pAsyncResult);

	// IUnknown
	STDMETHODIMP						QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG)				AddRef();
	STDMETHODIMP_(ULONG)				Release();

	//HRESULT	InitializeCapturer();
	//HRESULT	SetVideoWindow(long windowHandle, long left, long top, long right, long bottom);
	//HRESULT	put_VideoWindow(long left, long top, long right, long bottom);
	//HRESULT	StartCapture(int64_t durationInFrames, std::wstring captureFileFullPathNoExt);
	//HRESULT	StopCapture();
	//HRESULT	get_FramesCaptured(unsigned long* pVal);
	//HRESULT	get_FPSForCapture(long* pVal);
	//HRESULT	get_CaptureTimeRemaining(long* pVal);
	//HRESULT	PauseCapture(bool needToRestartPassthrough);
	//HRESULT	ResumeCapture(bool needToRestartPassthrough);
	//HRESULT	AbortCapture();
	//HRESULT	Shutdown();
	//HRESULT	get_ReceivingVideoSamples(bool* pVal);
	//HRESULT	SetPreviewOutputOn(bool inVal);
	//HRESULT	get_CaptureInputMode(long* pVal);
	//HRESULT	get_DeviceActive(bool* pVal);
	//HRESULT	get_PreviewIsOn(bool* pVal);
	//HRESULT	GetCaptureProgress(long* framesCaptured, long* durationFrames);
	//int64_t GetBytesWritten();
	//bool Is1080i();
	//bool GetForcedStop();
	//std::wstring GetName();

private:
	void								InitPassthrough(HWND videoWindow);
	void								InitCaptureAndPassthrough(HWND videoWindow, std::wstring captureFileName);
	void								StartSession();
	void								StopSession();
	void								OnTopologyReady();

	void								CreateCaptureSourceAndRenderers(HWND videoWindow);

	void								ProcessMediaEvent(CComPtr<IMFMediaEvent> mediaEvent);

	void								DumpTopologyFailed(CComPtr<IMFMediaEvent> mediaEvent);

	std::wstring								mVideoDeviceName = L"";
	std::wstring								mAudioDeviceName = L"";
	std::wstring								mCaptureFilePath = L"";

	std::shared_ptr<MediaSource>				mCaptureSource = nullptr;
	CComPtr<IMFActivate>						mVideoRenderer = nullptr;
	CComPtr<IMFActivate>						mAudioRenderer = nullptr;

	std::unique_ptr<Topology>					mTopology = nullptr;
		
	CComPtr<IMFMediaSession>					mMediaSession = nullptr;

	CComAutoCriticalSection						mCritSec;
	volatile long								mRefCount = 1;

	HANDLE										mStartedEvent = INVALID_HANDLE_VALUE;
	HANDLE										mPausedEvent = INVALID_HANDLE_VALUE;
	HANDLE										mStoppedEvent = INVALID_HANDLE_VALUE;

	bool										mCurrentlyPreviewing = false;
	bool										mCurrentlyCapturing = false;

	HWND										mVideoWindow = nullptr;
	CComPtr<IMFVideoDisplayControl>				mVideoDisplayControl = nullptr;
	CComPtr<IMFSimpleAudioVolume>				mAudioVolumeControl = nullptr;
};

CaptureMediaSession::CaptureMediaSession(std::wstring videoDeviceName, std::wstring audioDeviceName, std::wstring captureFilePath)
{
	m_pRep = std::make_unique<CaptureMediaSessionRep>(videoDeviceName, audioDeviceName, captureFilePath);
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
	mPausedEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mPausedEvent == INVALID_HANDLE_VALUE)
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
		if (eventType == MEVideoCaptureDeviceRemoved)
		{
			if (hrStatus == MF_E_AUDIO_RECORDING_DEVICE_INVALIDATED)
			{
				OutputDebugStringW(L"Audio USB Device disconnect!\n");
			}
			else if (hrStatus == MF_E_VIDEO_RECORDING_DEVICE_INVALIDATED)
			{
				OutputDebugStringW(L"Video USB Device disconnect!\n");
			}
		}
		return;
	}
	switch (eventType)
	{
	case MESessionTopologyStatus:
		OnERR_return(mediaEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus));
		if (TopoStatus == MF_TOPOSTATUS_READY)
		{
			OnTopologyReady();
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
	case MESessionPaused:
		OutputDebugStringW(L"MESessionPaused\n");
		SetEvent(mPausedEvent);
		break;
	default:
		OutputDebugStringW(L"UKNOWN IMFMediaEvent!\n");
		break;
	}
}

void CaptureMediaSessionRep::StartSession()
{
	OnERR_return(mMediaSession->BeginGetEvent((IMFAsyncCallback*)this, nullptr));

	PROPVARIANT varStart;
	PropVariantInit(&varStart);
	varStart.vt = VT_EMPTY;
	OnERR_return(mMediaSession->Start(&GUID_NULL, &varStart));
	DWORD res = WaitForSingleObject(mStartedEvent, INFINITE);
}

void CaptureMediaSessionRep::InitPassthrough(HWND videoWindow)
{
	mTopology.reset(nullptr);
	mTopology = std::make_unique<Topology>();
	CreateCaptureSourceAndRenderers(videoWindow);
	mTopology->CreateTopology(mCaptureSource, L"", mVideoRenderer, mAudioRenderer, mMediaSession);
}

void CaptureMediaSession::StartPreview(HWND videoWindow)
{
	m_pRep->StartPreview(videoWindow);
}
void CaptureMediaSessionRep::StartPreview(HWND videoWindow)
{
	if (mCurrentlyCapturing)
	{
		if (mAudioVolumeControl)
		{
			OnERR_return(mAudioVolumeControl->SetMute(FALSE));
		}
	}
	else
	{
		InitPassthrough(videoWindow);
		StartSession();
	}
	mCurrentlyPreviewing = true;
}

void CaptureMediaSession::StopPreview()
{
	m_pRep->StopPreview();
}
void CaptureMediaSessionRep::StopPreview()
{
	if (mCurrentlyCapturing)
	{
		if (mAudioVolumeControl)
		{
			OnERR_return(mAudioVolumeControl->SetMute(TRUE));
		}
	}
	else
	{
		StopSession();
	}
	mCurrentlyPreviewing = false;
}

void CaptureMediaSessionRep::InitCaptureAndPassthrough(HWND videoWindow, std::wstring captureFileName)
{
	mTopology.reset(nullptr);
	mTopology = std::make_unique<Topology>();
	CreateCaptureSourceAndRenderers(videoWindow);
	std::wstring fileToWrite = mCaptureFilePath + captureFileName;
	mTopology->CreateTopology(mCaptureSource, fileToWrite, mVideoRenderer, mAudioRenderer, mMediaSession);
}

void CaptureMediaSession::StartCapture(HWND videoWindow, std::wstring captureFileName)
{
	m_pRep->StartCapture(videoWindow, captureFileName);
}
void CaptureMediaSessionRep::StartCapture(HWND videoWindow, std::wstring captureFileName)
{
	InitCaptureAndPassthrough(videoWindow, captureFileName);
	StartSession();
	mCurrentlyPreviewing = true;
	mCurrentlyCapturing = true;
}

void CaptureMediaSession::StopCaptureImp()
{
	m_pRep->StopCaptureImp();
}
void CaptureMediaSessionRep::StopCaptureImp()
{
	StopSession();
	mCurrentlyPreviewing = false;
	mCurrentlyCapturing = false;
}

void CaptureMediaSessionRep::StopSession()
{
	OnERR_return(mMediaSession->Stop());
	DWORD res = WaitForSingleObject(mStoppedEvent, INFINITE);

	mMediaSession.Release();
	OnERR_return(MFCreateMediaSession(nullptr, &mMediaSession));
}

void CaptureMediaSession::PauseCapture()
{
	m_pRep->PauseCapture();
}
void CaptureMediaSessionRep::PauseCapture()
{
	if (mCurrentlyCapturing)
	{
		OnERR_return(mMediaSession->Pause());
		DWORD res = WaitForSingleObject(mPausedEvent, INFINITE);
	}
}

void CaptureMediaSession::ResumeCapture()
{
	m_pRep->ResumeCapture();
}
void CaptureMediaSessionRep::ResumeCapture()
{
	if (mCurrentlyCapturing)
	{
		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		varStart.vt = VT_EMPTY;
		OnERR_return(mMediaSession->Start(&GUID_NULL, &varStart));
		DWORD res = WaitForSingleObject(mStartedEvent, INFINITE);
	}
}

void CaptureMediaSessionRep::CreateCaptureSourceAndRenderers(HWND videoWindow)
{
	VideoDevices videoDevices(videoWindow);
	mVideoWindow = videoWindow;
	mVideoRenderer = videoDevices.GetVideoRenderer();

	AudioDevices audioDevices;
	mAudioRenderer = audioDevices.GetAudioRenderer();

	mCaptureSource = std::make_shared<MediaSource>(videoDevices.GetCaptureVideoDevice(mVideoDeviceName), audioDevices.GetCaptureAudioDevice(mAudioDeviceName));
}

void CaptureMediaSessionRep::OnTopologyReady()
{
	if (mVideoWindow)
	{
		mVideoDisplayControl.Release();
		OnERR_return(MFGetService(mMediaSession, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&mVideoDisplayControl));
		OnERR_return(mVideoDisplayControl->SetVideoWindow(mVideoWindow));
	}
	mAudioVolumeControl.Release();
	OnERR_return(MFGetService(mMediaSession, MR_POLICY_VOLUME_SERVICE, IID_IMFSimpleAudioVolume, (void**)&mAudioVolumeControl));
	OnERR_return(mAudioVolumeControl->SetMute(FALSE));
}

long long CaptureMediaSession::GetTime()
{
	return m_pRep->GetTime();
}
long long CaptureMediaSessionRep::GetTime()
{
	CComPtr<IMFClock> clock = nullptr;
	if (IsHRError(mMediaSession->GetClock(&clock)))
	{
		return 0;
	}
	long long clockTime = 0;
	long long systemTime = 0;
	if (IsHRError(clock->GetCorrelatedTime(0, &clockTime, &systemTime)))
	{
		return 0;
	}
	return clockTime;
}

HRESULT	CaptureMediaSession::InitializeCapturer()
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::SetVideoWindow(long windowHandle, long left, long top, long right, long bottom)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::put_VideoWindow(long left, long top, long right, long bottom)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::StartCapture(int64_t durationInFrames, std::wstring captureFileFullPathNoExt)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::StopCapture()
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::get_FramesCaptured(unsigned long* pVal)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::get_FPSForCapture(long* pVal)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::get_CaptureTimeRemaining(long* pVal)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::PauseCapture(bool needToRestartPassthrough)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::ResumeCapture(bool needToRestartPassthrough)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::AbortCapture()
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::Shutdown()
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::get_ReceivingVideoSamples(bool* pVal)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::SetPreviewOutputOn(bool inVal)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::get_CaptureInputMode(long* pVal)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::get_DeviceActive(bool* pVal)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::get_PreviewIsOn(bool* pVal)
{
	return E_FAIL;
}
HRESULT	CaptureMediaSession::GetCaptureProgress(long* framesCaptured, long* durationFrames)
{
	return E_FAIL;
}
int64_t CaptureMediaSession::GetBytesWritten()
{
	return 0;
}
bool CaptureMediaSession::Is1080i()
{
	return false;
}
bool CaptureMediaSession::GetForcedStop()
{
	return false;
}
std::wstring CaptureMediaSession::GetName()
{
	return L"";
}