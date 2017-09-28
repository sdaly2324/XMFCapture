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
	XMFCaptureEngineWrapper(IMFCaptureEngineOnEventCallback *pEventCallback, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, HANDLE hEvent, bool useOld);
	~XMFCaptureEngineWrapper();

	HRESULT SetupWriter(PCWSTR pszDestinationFile);

	HRESULT StartRecord();
	HRESULT StopRecord();

	HRESULT StartPreview(HWND hwnd);
	HRESULT StopPreview();
	
	HRESULT get_FramesCaptured(unsigned long* pVal) const;
	HRESULT	get_FPSForCapture(long* pVal) const;

private:
	XMFCaptureEngineWrapperRep* m_pRep;
};

