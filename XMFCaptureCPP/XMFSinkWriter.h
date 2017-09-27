#pragma once 

#include <windows.h>
#include <string>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

class XMFSinkWriterRep;

#ifdef XMFCAPTURECPP_EXPORTS
#define XMFCAPTURECPP_API __declspec(dllexport)
#else
#define XMFCAPTURECPP_API __declspec(dllimport)
#endif

class XMFCAPTURECPP_API XMFSinkWriter
{
public:
	XMFSinkWriter(LPCWSTR fullFilePath);
	~XMFSinkWriter();

	HRESULT WriteSample(DWORD dwStreamIndex, CComPtr<IMFSample> pSample, bool* bStopRequested);
	HRESULT AddStream(CComPtr<IMFMediaType> pTargetMediaType, DWORD* pdwStreamIndex);
	HRESULT SetInputMediaType(DWORD dwStreamIndex, CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFAttributes> pEncodingParameters);
	HRESULT BeginWriting();
	HRESULT EndWriting();
	HRESULT SignalAllStopped();
	HRESULT GetStatistics(DWORD dwStreamIndex, MF_SINK_WRITER_STATISTICS *pStats);

private:
	XMFSinkWriter();

	XMFSinkWriterRep* m_pRep;
};