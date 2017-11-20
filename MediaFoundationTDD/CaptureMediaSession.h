#pragma once

#include <windows.h>
#include <atlcomcli.h>
#include <memory>
#include <string>

#include "CaptureHAL.h"

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSession;
class OnTopologyReadyCallback
{
public:
	virtual void OnTopologyReady(CComPtr<IMFMediaSession> mediaSession) = 0;
};

class OnTopologyReadyCallback;
class CaptureMediaSessionRep;
class MediaFoundationTDD_API CaptureMediaSession : public CaptureHAL
{
public:
	CaptureMediaSession(std::wstring videoDeviceName, std::wstring audioDeviceName, std::wstring captureFilePath);
	~CaptureMediaSession();

	HRESULT								GetLastHRESULT();

	void								StartPreview(HWND videoWindow);
	void								StopPreview();
	void								StartCapture(HWND videoWindow, std::wstring captureFileName);
	void								PauseCapture();
	void								ResumeCapture();
	void								StopCaptureImp();
	long long							GetTime();


	HRESULT	InitializeCapturer();
	HRESULT	SetVideoWindow(long windowHandle, long left, long top, long right, long bottom);
	HRESULT	put_VideoWindow(long left, long top, long right, long bottom);
	HRESULT	StartCapture(int64_t durationInFrames, std::wstring captureFileFullPathNoExt);
	HRESULT	StopCapture();
	HRESULT	get_FramesCaptured(unsigned long* pVal);
	HRESULT	get_FPSForCapture(long* pVal);
	HRESULT	get_CaptureTimeRemaining(long* pVal);
	HRESULT	PauseCapture(bool needToRestartPassthrough);
	HRESULT	ResumeCapture(bool needToRestartPassthrough);
	HRESULT	AbortCapture();
	HRESULT	Shutdown();
	HRESULT	get_ReceivingVideoSamples(bool* pVal);
	HRESULT	SetPreviewOutputOn(bool inVal);
	HRESULT	get_CaptureInputMode(long* pVal);
	HRESULT	get_DeviceActive(bool* pVal);
	HRESULT	get_PreviewIsOn(bool* pVal);
	HRESULT	GetCaptureProgress(long* framesCaptured, long* durationFrames);
	int64_t GetBytesWritten();
	bool Is1080i();
	bool GetForcedStop();
	std::wstring GetName();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<CaptureMediaSessionRep> m_pRep = 0;
#pragma warning(pop)
};
