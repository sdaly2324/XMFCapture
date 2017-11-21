#include "CaptureMediaSession.h"
#include "MFUtils.h"
#include "Topology.h"
#include "VideoDevices.h"
#include "AudioDevices.h"
#include "MediaSource.h"
#include "MediaTypeUtils.h"

#include <mfapi.h>
#include <atlbase.h>
#include <mfidl.h>
#include <Mferror.h>
#include <evr.h>
#include <fstream>

const std::wstring gVideoDeviceName = L"XI100DUSB-SDI Video";		//<-------------------Video device to test-----------------------------
const std::wstring gAudioDeviceName = L"XI100DUSB-SDI Audio";		//<-------------------Audio device to test-----------------------------
//const std::wstring myVideoDeviceName = L"XI100DUSB-HDMI Video";	//<-------------------Video device to test-----------------------------
//const std::wstring myAudioDeviceName = L"XI100DUSB-HDMI Audio";	//<-------------------Audio device to test-----------------------------


class CaptureMediaSessionRep : public IMFAsyncCallback, public MFUtils
{
public:
	CaptureMediaSessionRep(std::wstring videoDeviceName, std::wstring audioDeviceName);
	virtual ~CaptureMediaSessionRep();

	HRESULT								GetLastHRESULT();
	
	HRESULT								StartCapture(int64_t durationInFrames, std::wstring captureFileFullPathNoExt);
	HRESULT								PauseCapture(bool needToRestartPassthrough);
	HRESULT								ResumeCapture();
	HRESULT								StopCapture();
	HRESULT								SetPreviewOutputOn(bool inVal);
	HRESULT								get_PreviewIsOn(bool* pVal);
	HRESULT								get_FPSForCapture(long* pVal);
	HRESULT								get_FramesCaptured(unsigned long* pVal);
	bool								GetForcedStop();
	HRESULT								get_CaptureInputMode(long* pVal);
	bool								Is1080i();
	HRESULT								GetCaptureProgress(long* framesCaptured, long* durationFrames);
	HRESULT								get_CaptureTimeRemaining(long* pVal);
	HRESULT								get_ReceivingVideoSamples(bool* pVal);
	HRESULT								SetVideoWindow(long windowHandle, long left, long top, long right, long bottom);
	HRESULT								put_VideoWindow(long left, long top, long right, long bottom);
	int64_t								GetBytesWritten();

	// IMFAsyncCallback
	STDMETHODIMP						GetParameters(DWORD* /*pdwFlags*/, DWORD* /*pdwQueue*/) { return E_NOTIMPL; }
	STDMETHODIMP						Invoke(IMFAsyncResult* pAsyncResult);

	// IUnknown
	STDMETHODIMP						QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG)				AddRef();
	STDMETHODIMP_(ULONG)				Release();

private:
	long long							GetTime();
	double								GetCurrrentVideoFormatFPS();
	void								StartPreview(HWND videoWindow);
	void								StopPreview();
	void								InitPassthrough(HWND videoWindow);
	void								InitCaptureAndPassthrough(HWND videoWindow, std::wstring captureFileFullPath);
	void								StartSession();
	void								StopSession();
	void								OnTopologyReady();

	void								CreateCaptureSourceAndRenderers(HWND videoWindow);

	void								ProcessMediaEvent(CComPtr<IMFMediaEvent> mediaEvent);

	void								DumpTopologyFailed(CComPtr<IMFMediaEvent> mediaEvent);

	std::wstring								mVideoDeviceName = L"";
	std::wstring								mAudioDeviceName = L"";

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

	int64_t										mDurationInFrames = 0;
	double										mFPSForCapture = 0;
	bool										mForcedStopped = false;
	std::wstring								mCaptureFileFullPath = L"";
};

CaptureMediaSession::CaptureMediaSession()
{

}

HRESULT	CaptureMediaSession::InitializeCapturer()
{
	m_pRep = std::make_unique<CaptureMediaSessionRep>(gVideoDeviceName, gAudioDeviceName);
	return S_OK;
}
CaptureMediaSessionRep::CaptureMediaSessionRep(std::wstring videoDeviceName, std::wstring audioDeviceName):
	mVideoDeviceName(videoDeviceName),
	mAudioDeviceName(audioDeviceName)
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
			if (mCurrentlyCapturing)
			{
				StopCapture();
				mForcedStopped = true;
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

void CaptureMediaSessionRep::InitCaptureAndPassthrough(HWND videoWindow, std::wstring captureFileFullPath)
{
	mCaptureFileFullPath = captureFileFullPath;
	mTopology.reset(nullptr);
	mTopology = std::make_unique<Topology>();
	CreateCaptureSourceAndRenderers(videoWindow);
	mTopology->CreateTopology(mCaptureSource, mCaptureFileFullPath, mVideoRenderer, mAudioRenderer, mMediaSession);
}

void CaptureMediaSessionRep::StopSession()
{
	OnERR_return(mMediaSession->Stop());
	DWORD res = WaitForSingleObject(mStoppedEvent, INFINITE);

	mMediaSession.Release();
	OnERR_return(MFCreateMediaSession(nullptr, &mMediaSession));
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


HRESULT	CaptureMediaSession::SetVideoWindow(long windowHandle, long left, long top, long right, long bottom)
{
	return m_pRep->SetVideoWindow(windowHandle, left, top, right, bottom);
}
HRESULT	CaptureMediaSessionRep::SetVideoWindow(long windowHandle, long left, long top, long right, long bottom)
{
	mVideoWindow = (HWND)windowHandle;
	return S_OK;
}

HRESULT	CaptureMediaSession::put_VideoWindow(long left, long top, long right, long bottom)
{
	return m_pRep->put_VideoWindow(left, top, right, bottom);
}
HRESULT	CaptureMediaSessionRep::put_VideoWindow(long left, long top, long right, long bottom)
{
	HRESULT hr = S_FALSE;
	if (mVideoWindow)
	{
		// use SWP_NOSIZE
		// a resize trigger in the C# code that then called this is what got us here
		// don't trigger another resize or this becomes recursive
		BOOL retVal = ::SetWindowPos(mVideoWindow, NULL, left, top, right - left, bottom - top, SWP_NOOWNERZORDER | SWP_NOSIZE);
		if (retVal == FALSE)
		{
			OutputDebugStringA("CaptureMediaSessionRep::put_VideoWindow SetWindowPos FAILED!\n");
		}
		else if (mVideoDisplayControl)
		{
			RECT displayRect = { 0, 0, right - left, bottom - top };
			hr = mVideoDisplayControl->SetVideoPosition(NULL, &displayRect);
		}
	}
	return hr;
}

HRESULT	CaptureMediaSession::StartCapture(int64_t durationInFrames, std::wstring captureFileFullPathNoExt)
{
	return m_pRep->StartCapture(durationInFrames, captureFileFullPathNoExt);
}
HRESULT CaptureMediaSessionRep::StartCapture(int64_t durationInFrames, std::wstring captureFileFullPathNoExt)
{
	mForcedStopped = false;
	std::wstring captureFileFullPath = captureFileFullPathNoExt + L".ts";
	InitCaptureAndPassthrough(mVideoWindow, captureFileFullPath);
	StartSession();
	mCurrentlyPreviewing = true;
	mCurrentlyCapturing = true;
	return GetLastHRESULT();
}

HRESULT	CaptureMediaSession::StopCapture()
{
	return m_pRep->StopCapture();
}
HRESULT CaptureMediaSessionRep::StopCapture()
{
	StopSession();
	mCurrentlyPreviewing = false;
	mCurrentlyCapturing = false;
	return GetLastHRESULT();
}

HRESULT	CaptureMediaSession::get_FramesCaptured(unsigned long* pVal)
{
	return m_pRep->get_FramesCaptured(pVal);
}
HRESULT	CaptureMediaSessionRep::get_FramesCaptured(unsigned long* pVal)
{
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	double curTimeInSeconds = (double)GetTime() / 10000000;
	double fps = GetCurrrentVideoFormatFPS();
	*pVal = (unsigned long)(curTimeInSeconds * fps);
	return GetLastHRESULT();
}

HRESULT	CaptureMediaSession::get_FPSForCapture(long* pVal)
{
	return m_pRep->get_FPSForCapture(pVal);
}
HRESULT	CaptureMediaSessionRep::get_FPSForCapture(long* pVal)
{
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	if (mFPSForCapture)
	{
		*pVal = (long)mFPSForCapture;
		return S_OK;
	}
	mFPSForCapture = GetCurrrentVideoFormatFPS();
	*pVal = (long)mFPSForCapture; //?????? CHECK ME ??????
	return GetLastHRESULT();
}

double CaptureMediaSessionRep::GetCurrrentVideoFormatFPS()
{
	if (mFPSForCapture)
	{
		return mFPSForCapture;
	}
	if (!mCaptureSource)
	{
		SetLastHR_Fail();
		return 0;
	}
	CComPtr<IMFMediaType> mediaType = mCaptureSource->GetVideoMediaType();
	if (!mediaType)
	{
		SetLastHR_Fail();
		return 0;
	}
	UINT32 frameRatenumerator = 0;
	UINT32 frameRateDenominator = 0;
	if (IsHRError(MFGetAttributeRatio(mediaType, MF_MT_FRAME_RATE, &frameRatenumerator, &frameRateDenominator)))
	{
		SetLastHR_Fail();
		return 0;
	}
	return (double)frameRatenumerator / (double)frameRateDenominator;
}

HRESULT	CaptureMediaSession::get_CaptureTimeRemaining(long* pVal)
{
	return m_pRep->get_CaptureTimeRemaining(pVal);
}
HRESULT	CaptureMediaSessionRep::get_CaptureTimeRemaining(long* pVal)
{
	HRESULT hr = S_OK;
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	unsigned long framesCaptured = 0;
	hr = get_FramesCaptured(&framesCaptured);
	if (IsHRError(hr))
	{
		return hr;
	}
	double fps = 0;
	fps = GetCurrrentVideoFormatFPS();
	if (fps > 0)
	{
		*pVal = (long)(((double)mDurationInFrames - (double)framesCaptured) / fps);
	}
	return GetLastHRESULT();
}

HRESULT	CaptureMediaSession::PauseCapture(bool needToRestartPassthrough)
{
	return m_pRep->PauseCapture(needToRestartPassthrough);
}
HRESULT CaptureMediaSessionRep::PauseCapture(bool needToRestartPassthrough)
{
	if (mCurrentlyCapturing)
	{
		OnERR_return_HR(mMediaSession->Pause());
		DWORD res = WaitForSingleObject(mPausedEvent, INFINITE);
		if (needToRestartPassthrough)
		{
			StartPreview(mVideoWindow);
		}
	}
	return GetLastHRESULT();
}

HRESULT	CaptureMediaSession::ResumeCapture(bool needToRestartPassthrough)
{
	return m_pRep->ResumeCapture();
}
HRESULT CaptureMediaSessionRep::ResumeCapture()
{
	if (mCurrentlyCapturing)
	{
		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		varStart.vt = VT_EMPTY;
		OnERR_return_HR(mMediaSession->Start(&GUID_NULL, &varStart));
		DWORD res = WaitForSingleObject(mStartedEvent, INFINITE);
	}
	return GetLastHRESULT();
}
HRESULT	CaptureMediaSession::AbortCapture()
{
	return m_pRep->StopCapture();
}
HRESULT	CaptureMediaSession::Shutdown()
{
	return m_pRep->StopCapture();
}

HRESULT	CaptureMediaSession::get_ReceivingVideoSamples(bool* pVal)
{
	return m_pRep->get_ReceivingVideoSamples(pVal);
}
HRESULT	CaptureMediaSessionRep::get_ReceivingVideoSamples(bool* pVal)
{
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	if (mCurrentlyPreviewing)
	{
		*pVal = true;
		return S_OK;
	}

	unsigned long cap = 0;
	HRESULT hr = get_FramesCaptured(&cap);
	if (!IsHRError(hr))
	{
		if (cap > 0)
		{
			*pVal = true;
		}
		else
		{
			*pVal = false;
		}
	}
	return hr;
}

HRESULT	CaptureMediaSession::SetPreviewOutputOn(bool inVal)
{
	return m_pRep->SetPreviewOutputOn(inVal);
}
HRESULT CaptureMediaSessionRep::SetPreviewOutputOn(bool inVal)
{
	if (inVal)
	{
		StartPreview(mVideoWindow);
	}
	else
	{
		StopPreview();
	}
	return GetLastHRESULT();
}

HRESULT	CaptureMediaSession::get_CaptureInputMode(long* pVal)
{
	return m_pRep->get_CaptureInputMode(pVal);
}
HRESULT	CaptureMediaSessionRep::get_CaptureInputMode(long* pVal)
{
	if (!mCaptureSource)
	{
		SetLastHR_Fail();
		return 0;
	}
	CComPtr<IMFMediaType> mediaType = mCaptureSource->GetVideoMediaType();
	if (!mediaType)
	{
		SetLastHR_Fail();
		return 0;
	}
	if (!pVal)
	{
		SetLastHR_Fail();
		return 0;
	}
	*pVal = MediaTypeUtils::ConvertVideoMediaTypeToCaptureInputMode(mediaType);
	return GetLastHRESULT();
}

HRESULT	CaptureMediaSession::get_DeviceActive(bool* pVal)
{
	if (!pVal)
	{
		return E_POINTER;
	}
	*pVal = true;
	return S_OK;
}

HRESULT	CaptureMediaSession::get_PreviewIsOn(bool* pVal)
{
	return m_pRep->get_PreviewIsOn(pVal);
}
HRESULT	CaptureMediaSessionRep::get_PreviewIsOn(bool* pVal)
{
	if (!pVal)
	{
		return E_INVALIDARG;
	}
	*pVal = mCurrentlyPreviewing;
	return S_OK;
}

HRESULT	CaptureMediaSession::GetCaptureProgress(long* framesCaptured, long* durationFrames)
{
	return m_pRep->GetCaptureProgress(framesCaptured, durationFrames);
}
HRESULT	CaptureMediaSessionRep::GetCaptureProgress(long* framesCaptured, long* durationFrames)
{
	HRESULT hr = get_FramesCaptured((unsigned long*)framesCaptured);
	if (IsHRError(hr))
	{
		hr = get_CaptureTimeRemaining(durationFrames);
	}
	return hr;
}

int64_t CaptureMediaSession::GetBytesWritten()
{
	return m_pRep->GetBytesWritten();
}
int64_t CaptureMediaSessionRep::GetBytesWritten()
{
	std::ifstream lstream(mCaptureFileFullPath, std::ifstream::ate | std::ifstream::binary);
	lstream.seekg(0, std::ios_base::end);
	return lstream.tellg();
}

bool CaptureMediaSession::Is1080i()
{
	return m_pRep->Is1080i();
}
bool CaptureMediaSessionRep::Is1080i()
{
	long inputMode = 0;
	if (IsHRError(get_CaptureInputMode(&inputMode)))
	{
		return false;
	}
	if (inputMode == captureInputModeHD1080i50 ||
		inputMode == captureInputModeHD1080i5994 ||
		inputMode == captureInputModeHD1080i6000)
	{
		return true;
	}
	return false;
}

bool CaptureMediaSession::GetForcedStop()
{
	return m_pRep->GetForcedStop();
}
bool CaptureMediaSessionRep::GetForcedStop()
{
	return mForcedStopped;
}

std::wstring CaptureMediaSession::GetName()
{
	return L"CaptureMediaSession";
}