#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>
#include <string>

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
class MediaFoundationTDD_API CaptureMediaSession
{
public:
	CaptureMediaSession(std::wstring videoDeviceName, std::wstring audioDeviceName, std::wstring captureFilePath);
	~CaptureMediaSession();

	HRESULT								GetLastHRESULT();

	void								StartPreview(HWND videoWindow);
	void								StopPreview();
	void								StartCapture(HWND videoWindow, std::wstring captureFileName);
	void								StopCapture();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<CaptureMediaSessionRep> m_pRep = 0;
#pragma warning(pop)
};
