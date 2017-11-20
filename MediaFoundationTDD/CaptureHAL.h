#pragma once
#include <winnt.h>
#include <string>
class CaptureHAL
{
public:
	virtual HRESULT	InitializeCapturer() = 0;
	virtual HRESULT	SetVideoWindow(long windowHandle, long left, long top, long right, long bottom) = 0;
	virtual HRESULT	put_VideoWindow(long left, long top, long right, long bottom) = 0;
	virtual HRESULT	StartCapture(int64_t durationInFrames, std::wstring captureFileFullPathNoExt) = 0;
	virtual HRESULT	StopCapture() = 0;
	virtual HRESULT	get_FramesCaptured(unsigned long* pVal) = 0;
	virtual HRESULT	get_FPSForCapture(long* pVal) = 0;
	virtual HRESULT	get_CaptureTimeRemaining(long* pVal) = 0;
	virtual HRESULT	PauseCapture(bool needToRestartPassthrough) = 0;
	virtual HRESULT	ResumeCapture(bool needToRestartPassthrough) = 0;
	virtual HRESULT	AbortCapture() = 0;
	virtual HRESULT	Shutdown() = 0;
	virtual HRESULT	get_ReceivingVideoSamples(bool* pVal) = 0;
	virtual HRESULT	SetPreviewOutputOn(bool inVal) = 0;
	virtual HRESULT	get_CaptureInputMode(long* pVal) = 0;
	virtual HRESULT	get_DeviceActive(bool* pVal) = 0;
	virtual HRESULT	get_PreviewIsOn(bool* pVal) = 0;
	virtual HRESULT	GetCaptureProgress(	long* framesCaptured, long* durationFrames) = 0;
	virtual int64_t GetBytesWritten() = 0;
	virtual bool Is1080i() = 0;
	virtual bool GetForcedStop() = 0;
	virtual std::wstring GetName() = 0;
};