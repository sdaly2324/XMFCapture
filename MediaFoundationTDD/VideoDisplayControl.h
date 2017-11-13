#pragma once

#include <windows.h>
#include <atlcomcli.h>
#include <memory>

#include "CaptureMediaSession.h"

struct IMFMediaSession;
struct IMFVideoDisplayControl;
class VideoDisplayControlRep;
class VideoDisplayControl : public OnTopologyReadyCallback
{
public:
	VideoDisplayControl(HWND windowForVideo);
	~VideoDisplayControl();

	//OnTopologyReadyCallback
	virtual void						OnTopologyReady(CComPtr<IMFMediaSession> mediaSession);

	HRESULT								GetLastHRESULT();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<VideoDisplayControlRep> m_pRep = 0;
#pragma warning(pop)
};