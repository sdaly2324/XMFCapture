#pragma once

#include <windows.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>
#include <atlcom.h>

#include "XOSMFCaptureDevice.h"

class XOSMFAVSourceReader;
class XOSMFSinkWriter;

#ifdef XOSMFCAPTURECPP_EXPORTS
#define XOSMFCAPTURECPP_API __declspec(dllexport)
#else
#define XOSMFCAPTURECPP_API __declspec(dllimport)
#endif

class XOSMFCAPTURECPP_API XOSMFSourceManager
{
public:
	XOSMFSourceManager(XOSMFCaptureDevice* pAudioDevice, XOSMFCaptureDevice* pVideoDevice, XOSMFSinkWriter* pXOSMFSinkWriter);
	virtual ~XOSMFSourceManager();

	HRESULT Start();
	HRESULT GetVideoInputMediaType(CComPtr<IMFMediaType>& pVideoInputMediaType);
	HRESULT GetAudioInputMediaType(CComPtr<IMFMediaType>& pAudioInputMediaType);
	HRESULT GetXOSMFAVSourceReader(XOSMFAVSourceReader** ppXOSMFAVSourceReader);
	bool Capturing();

private:
	HRESULT CreateAggregateMediaSource(CComPtr<IMFMediaSource> videoSource, CComPtr<IMFMediaSource> audioSource, CComPtr<IMFMediaSource>& pVASource);

	XOSMFAVSourceReader*		m_pXOSMFAVSourceReader;
};
