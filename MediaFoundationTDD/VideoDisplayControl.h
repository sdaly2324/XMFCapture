#pragma once

#include <windows.h>
#include <atlcomcli.h>
#include <memory>

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

	void SetIsParticipating(bool isParticipating);

	//OnTopologyReadyCallback
	virtual void						OnTopologyReady(CComPtr<IMFMediaSession> mediaSession);

	HRESULT								GetLastHRESULT();
	CComPtr<IMFVideoDisplayControl>		GetVideoDisplayControl();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<VideoDisplayControlRep> m_pRep = 0;
#pragma warning(pop)
};