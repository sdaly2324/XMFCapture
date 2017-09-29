#pragma once
#include <atlbase.h>
#include <mfidl.h>
#include <mfreadwrite.h>

struct IMFMediaType;
class IXMFCaptureEngine
{
public:
	virtual HRESULT SetupWriter(PCWSTR pszDestinationFile) = 0;
	virtual CComPtr<IMFMediaType> GetAudioMTypeFromSource() = 0;
	virtual CComPtr<IMFMediaType> GetVideoMTypeFromSource() = 0;
	virtual HRESULT AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType) = 0;
	virtual HRESULT AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType) = 0;

	virtual HRESULT StartRecord() = 0;
	virtual HRESULT StopRecord() = 0;

	virtual HRESULT StartPreview(HWND hwnd) = 0;
	virtual HRESULT StopPreview() = 0;

	virtual bool IsPreviewing() const = 0;
	virtual bool IsRecording() const = 0;
	virtual HRESULT GetStatistics(MF_SINK_WRITER_STATISTICS* pStats) = 0;
};