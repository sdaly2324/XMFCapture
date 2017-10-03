#pragma once

#include <windows.h>
#include <vector>
#include <memory>
#include <Dbt.h>

#include "XMFUtilities.h"

#ifdef XMFCAPTURECPP_EXPORTS
#define XMFCAPTURECPP_API __declspec(dllexport)
#else
#define XMFCAPTURECPP_API __declspec(dllimport)
#endif

class XMFCaptureAPIRep;
class XMFCAPTURECPP_API XMFCaptureAPI
{
public:
	static XMFCaptureAPI* GetInstance();
	virtual ~XMFCaptureAPI();

	HRESULT			ReEnumerateDevices();

	XOSStringList	GetDevicePairNamesList();
	void			SetCurrentDevice(XOSString deviceName);
	XOSString		GetCurrentDevice();

	void			SetOutputPath(XOSString outputPath);

	bool			IsCapturing();
	HRESULT			CheckDeviceLost(DEV_BROADCAST_HDR* pHdr, bool* pbDeviceLost);

	HRESULT			StartCapture(HWND hwnd, bool useOld);
	HRESULT			StopCapture();

	HRESULT			StartPreview(HWND hwnd);
	HRESULT			StopPreview();
	HRESULT			IsPreviewRunning();

	HRESULT			get_FramesCaptured(unsigned long* pVal);
	HRESULT			get_FPSForCapture(long* pVal);
	HRESULT			get_PreviewIsOn(bool* pVal);
	void			SetVideoDisplaySize(long width, long height);

private:
	XMFCaptureAPI();
	XMFCaptureAPIRep* m_pRep;
};

