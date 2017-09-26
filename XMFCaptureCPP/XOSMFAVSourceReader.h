#pragma once

#include <windows.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>
#include <atlcom.h>

#define DEBUG_TIMMING

class XOSMFAVSourceReaderRep;
class XOSMFSinkWriter;
class XOSMFAVSourceReader
{
public:
	friend class XOSMFSourceReaderCallback;
	XOSMFAVSourceReader(XOSMFSinkWriter* pXOSMFSinkWriter, CComPtr<IMFMediaSource> pVASource);
	virtual ~XOSMFAVSourceReader();

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
	XOSMFAVSourceReader();

	XOSMFAVSourceReaderRep* m_pRep;
};
