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

class XMFPreviewRep;
class XMFCaptureDevice;
class XMFCAPTURECPP_API XMFPreview
{
public:
	XMFPreview();
	virtual ~XMFPreview();

	virtual HRESULT			StartPreview(HWND hWindowForVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceAudio);
	virtual HRESULT			StopPreview();
	void					SetVideoDisplaySize(long width, long height);

private:
	XMFPreviewRep* m_RepPtr;
};

