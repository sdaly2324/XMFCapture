#pragma once
#include <memory>

#include "IXMFCaptureEngine.h"
#include "XMFCaptureDevice.h"

class XMFCaptureUsingIMFCaptureEngineRep;
class XMFCaptureUsingIMFCaptureEngine : public IXMFCaptureEngine
{
public:
	XMFCaptureUsingIMFCaptureEngine(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice);
	~XMFCaptureUsingIMFCaptureEngine();

	HRESULT SetupWriter(PCWSTR pszDestinationFile);
	CComPtr<IMFMediaType> GetAudioMTypeFromSource();
	CComPtr<IMFMediaType> GetVideoMTypeFromSource();
	HRESULT AddVideoStream(CComPtr<IMFMediaType> pVideoOutputMediaType);
	HRESULT AddAudioStream(CComPtr<IMFMediaType> pAudioOutputMediaType);

	HRESULT StartRecord(HWND hwnd);
	HRESULT StopRecord();

	HRESULT StartPreview(HWND hwnd);
	HRESULT StopPreview();

	virtual bool IsPreviewing() const;
	virtual bool IsRecording() const;
	HRESULT GetStatistics(MF_SINK_WRITER_STATISTICS* pStats);

private:
	XMFCaptureUsingIMFCaptureEngineRep* m_pRep;
};

