#pragma once 

#include <windows.h>
#include <string>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

class XOSMFSinkWriterRep;

#ifdef XOSMFCAPTURECPP_EXPORTS
#define XOSMFCAPTURECPP_API __declspec(dllexport)
#else
#define XOSMFCAPTURECPP_API __declspec(dllimport)
#endif

class XOSMFCAPTURECPP_API XOSMFSinkWriter
{
public:
	XOSMFSinkWriter(LPCWSTR fullFilePath);
	~XOSMFSinkWriter();

	HRESULT WriteSample(DWORD dwStreamIndex, CComPtr<IMFSample> pSample, bool* bStopRequested);
	HRESULT AddStream(CComPtr<IMFMediaType> pTargetMediaType, DWORD* pdwStreamIndex);
	HRESULT SetInputMediaType(DWORD dwStreamIndex, CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFAttributes> pEncodingParameters);
	HRESULT BeginWriting();
	HRESULT EndWriting();
	HRESULT SignalAllStopped();
	HRESULT GetStatistics(DWORD dwStreamIndex, MF_SINK_WRITER_STATISTICS *pStats);

private:
	XOSMFSinkWriter();

	XOSMFSinkWriterRep* m_pRep;
};