#pragma once

#include <windows.h>
#include <string>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

class XMFSinkWriterCallback : public IMFSinkWriterCallback
{
public:
	XMFSinkWriterCallback();
	virtual ~XMFSinkWriterCallback();

	// IMFSinkWriterCallback methods
	STDMETHODIMP OnFinalize(HRESULT hrStatus);
	STDMETHODIMP OnMarker(DWORD dwStreamIndex, LPVOID pvContext);

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

private:
	long m_nRefCount;
};