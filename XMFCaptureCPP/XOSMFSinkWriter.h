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

	HRESULT ConfigureAudioEncoder(CComPtr<IMFMediaType> pAudioReaderInputMediaType, DWORD* pdwSinkStreamIndex);
	HRESULT ConfigureVideoEncoder(CComPtr<IMFMediaType> pVideoReaderInputMediaType, DWORD* pdwSinkStreamIndex);
	HRESULT WriteSample(DWORD dwStreamIndex, CComPtr<IMFSample> pSample, bool* bStopRequested);
	HRESULT AddStream(CComPtr<IMFMediaType> pTargetMediaType, DWORD* pdwStreamIndex);
	HRESULT SetInputMediaType(DWORD dwStreamIndex, CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFAttributes> pEncodingParameters);
	HRESULT BeginWriting();
	HRESULT EndWriting();
	HRESULT SignalAllStopped();

private:
	XOSMFSinkWriter();

	XOSMFSinkWriterRep* m_pRep;
};