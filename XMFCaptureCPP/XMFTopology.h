#pragma once

#include <windows.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

#ifdef MFCAPTURE_TOPOLOGY_EXPORTS
#define MFCAPTURE_TOPOLOGY_API __declspec(dllexport)
#else
#define MFCAPTURE_TOPOLOGY_API __declspec(dllimport)
#endif

class XOSMFAVSourceReader;
class XOSMFSinkWriter;
class XOSMFTopologyRep;
class MFCAPTURE_TOPOLOGY_API XOSMFTopology
{
public:
	XOSMFTopology(XOSMFAVSourceReader* pXOSMFAVSourceReader, bool previewOnly, HWND hwndVideo, XOSMFSinkWriter* pXOSMFSinkWriter);
	~XOSMFTopology();

	HRESULT Stop();

private:
	XOSMFTopologyRep* m_pRep;
};