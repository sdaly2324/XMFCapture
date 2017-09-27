#pragma once

#include <windows.h>
#include <string>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

class XMFAVSourceReader;
class XMFSourceReaderCallback : public IMFSourceReaderCallback
{
public:
	XMFSourceReaderCallback(XMFAVSourceReader* pXMFAVSourceReader);
	~XMFSourceReaderCallback();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFSourceReaderCallback methods
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample* pSample);
	STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent* pEvent);
	STDMETHODIMP OnFlush(DWORD dwStreamIndex);

private:
	long m_nRefCount;
	XMFAVSourceReader* m_pXMFAVSourceReader;
};