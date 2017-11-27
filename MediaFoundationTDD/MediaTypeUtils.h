#pragma once
#include "MFUtils.h"

#include <windows.h>
#include <atlcomcli.h>
#include <memory>

enum CaptureInputMode // from DeckLinkAPI_h.h
{
	captureInputModeNTSC = 0x6e747363,
	captureInputModeNTSC2398 = 0x6e743233,
	captureInputModePAL = 0x70616c20,
	captureInputModeNTSCp = 0x6e747370,
	captureInputModePALp = 0x70616c70,
	captureInputModeHD1080p2398 = 0x32337073,
	captureInputModeHD1080p24 = 0x32347073,
	captureInputModeHD1080p25 = 0x48703235,
	captureInputModeHD1080p2997 = 0x48703239,
	captureInputModeHD1080p30 = 0x48703330,
	captureInputModeHD1080i50 = 0x48693530,
	captureInputModeHD1080i5994 = 0x48693539,
	captureInputModeHD1080i6000 = 0x48693630,
	captureInputModeHD1080p50 = 0x48703530,
	captureInputModeHD1080p5994 = 0x48703539,
	captureInputModeHD1080p6000 = 0x48703630,
	captureInputModeHD720p50 = 0x68703530,
	captureInputModeHD720p5994 = 0x68703539,
	captureInputModeHD720p60 = 0x68703630,
	captureInputMode2k2398 = 0x326b3233,
	captureInputMode2k24 = 0x326b3234,
	captureInputMode2k25 = 0x326b3235,
	captureInputModeUnknown = 0x69756e6b
};

struct IMFCollection;
struct IMFMediaType;
struct IMFAttributes;
class MediaTypeUtils : public MFUtils
{
public:
	MediaTypeUtils();
	static HRESULT					GetLastHRESULT();

	static CComPtr<IMFMediaType>	CreateAudioEncodingMediaType();
	static CComPtr<IMFMediaType>	CreateAudioInputMediaType();
	static CComPtr<IMFMediaType>	CreateVideoEncodingMediaType(CComPtr<IMFAttributes> inAttrs);
	static CComPtr<IMFMediaType>	CreateVideoNV12MediaType(CComPtr<IMFAttributes> inAttrs);
	static CaptureInputMode			ConvertVideoMediaTypeToCaptureInputMode(CComPtr<IMFMediaType> mediaType);
	static std::wstring				ConvertCaptureInputModeToString(CaptureInputMode mode);
	static bool						Is720(CComPtr<IMFMediaType> mediaType);
	static bool						IsYUY2(CComPtr<IMFMediaType> mediaType);
	static bool						Is5994(CComPtr<IMFMediaType> mediaType);
private:
	void DumpAvailableAACFormats(CComPtr<IMFCollection> availableTypes);
	template <class T>
	static HRESULT GetCollectionObject(CComPtr<IMFCollection> collection, DWORD index, T **object)
	{
		CComPtr<IUnknown> unknown;
		HRESULT hr = collection->GetElement(index, &unknown);
		if (SUCCEEDED(hr))
		{
			hr = unknown->QueryInterface(IID_PPV_ARGS(object));
		}
		return hr;
	}
};