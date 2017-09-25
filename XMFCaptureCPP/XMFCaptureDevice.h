#pragma once

#include <memory>
#include <vector>
#include <atlbase.h>
#include <Dbt.h>

#include "XMFUtilities.h"

class XMFFormat;
struct IMFActivate;
struct IMFStreamDescriptor;
struct IMFMediaTypeHandler;
struct IMFMediaType;
struct IMFMediaSource;
struct IMFPresentationDescriptor;
class XMFCaptureDeviceRep;

class XMFCaptureDevice
{
public:
	XMFCaptureDevice(CComPtr<IMFActivate> pMFActivate, GUID type);
	virtual ~XMFCaptureDevice();

	WCHAR* GetDeviceName();
	 
	bool SupportsAudio();
	bool SupportsVideo();
	bool SupportsAudioAndVideo();
	bool operator==(const XMFCaptureDevice& obj);

	HRESULT GetIMFMediaSource(CComPtr<IMFMediaSource>& pMFMediaSource);
	HRESULT GetIMFActivate(CComPtr<IUnknown>& pIMFActivate);
	HRESULT StartCapture();
	HRESULT CheckDeviceLost(DEV_BROADCAST_HDR* pHdr, bool* pbDeviceLost);
	void StopCapture();

	// true if any one format in the list is a full match
	bool SupportsAnyOfTheseFormats(std::vector<std::shared_ptr<XMFFormat>> validFormats);

private:
	XMFCaptureDevice();

	XMFCaptureDeviceRep* m_RepPtr;
};