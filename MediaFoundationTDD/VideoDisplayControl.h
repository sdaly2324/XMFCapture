#pragma once

#include <windows.h>
#include <atlcomcli.h>

#include "MediaSession.h"

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSession;
struct IMFVideoDisplayControl;
class VideoDisplayControlRep;
class MediaFoundationTDD_API VideoDisplayControl : public OnTopologyReadyCallback
{
public:
	VideoDisplayControl();
	~VideoDisplayControl();

	//OnTopologyReadyCallback
	virtual void						OnTopologyReady(CComPtr<IMFMediaSession> mediaSession);

	HRESULT								GetLastHRESULT();
	CComPtr<IMFVideoDisplayControl>		GetVideoDisplayControl();

private:
	VideoDisplayControlRep* m_pRep = 0;
};