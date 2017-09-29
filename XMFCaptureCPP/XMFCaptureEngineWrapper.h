#pragma once
#include <memory>
#include <atlbase.h>

struct IMFCaptureEngineOnEventCallback;
class XMFCaptureDevice;
class XMFCaptureEngineWrapperRep;
struct IMFCaptureSink;
struct IMFMediaType;
struct IMFCaptureSource;
class XMFCaptureEngineWrapper
{
public:
	XMFCaptureEngineWrapper(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld);
	~XMFCaptureEngineWrapper();

	HRESULT StartRecord(PCWSTR pszDestinationFile);
	HRESULT StopRecord();

	HRESULT StartPreview(HWND hwnd);
	HRESULT StopPreview();
	
	bool IsPreviewing() const;
	bool IsRecording() const;
	HRESULT get_FramesCaptured(unsigned long* pVal) const;
	HRESULT	get_FPSForCapture(long* pVal) const;

private:
	XMFCaptureEngineWrapperRep* m_pRep;
};

