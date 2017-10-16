#pragma once
#include <windows.h>
#include <atlcomcli.h>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSession;
class OnTopologyReadyCallback
{
public:
	virtual void OnTopologyReady(CComPtr<IMFMediaSession> mediaSession) = 0;
};

class OnTopologyReadyCallback;
class MediaSessionRep;
class MediaFoundationTDD_API MediaSession
{
public:
	MediaSession(OnTopologyReadyCallback* onTopologyReadyCallback);
	~MediaSession();

	HRESULT								GetLastHRESULT();

	void								Start();
	void								Stop();

	CComPtr<IMFMediaSession>			GetMediaSession();

private:
	MediaSession();
	MediaSessionRep* m_pRep = 0;
};
