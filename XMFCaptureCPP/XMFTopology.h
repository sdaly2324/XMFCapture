#pragma once

#include <windows.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

class XMFAVSourceReader;
class XMFSinkWriter;
class XMFTopologyRep;
class XMFTopology
{
public:
	XMFTopology(XMFAVSourceReader* pXMFAVSourceReader, HWND hwndVideo, XMFSinkWriter* pXMFSinkWriter);
	~XMFTopology();

private:
	XMFTopologyRep* m_pRep;
};