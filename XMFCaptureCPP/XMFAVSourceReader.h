#pragma once

#include <windows.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>
#include <atlcom.h>

#define DEBUG_TIMMING

class XMFAVSourceReaderRep;
class XMFSinkWriter;
class XMFAVSourceReader
{
public:
	friend class XMFSourceReaderCallback;
	XMFAVSourceReader(XMFSinkWriter* pXMFSinkWriter, CComPtr<IMFMediaSource> pVASource);
	virtual ~XMFAVSourceReader();

	HRESULT Start();
	HRESULT GetVideoInputMediaType(CComPtr<IMFMediaType>& pVideoReaderInputMediaTypeCurrent);
	HRESULT GetAudioInputMediaType(CComPtr<IMFMediaType>& pAudioInputMediaType);
	HRESULT GetPresentationDescriptor(CComPtr<IMFPresentationDescriptor>& pVideoInputPresentationDescriptor);
	HRESULT GetMFMediaSource(CComPtr<IMFMediaSource>& pMFMediaSource);
	bool Capturing();

protected:
	HRESULT OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimeStamp, CComPtr<IMFSample> pMFSample);
	HRESULT OnFlush(DWORD dwStreamIndex);
private:
	XMFAVSourceReader();

	XMFAVSourceReaderRep* m_pRep;
};
