#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>

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
	MediaSession(std::shared_ptr<OnTopologyReadyCallback> onTopologyReadyCallback);
	~MediaSession();

	HRESULT								GetLastHRESULT();

	void								Start();
	void								Stop();

	CComPtr<IMFMediaSession>			GetMediaSession();

private:
	MediaSession();
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<MediaSessionRep> m_pRep = 0;
#pragma warning(pop)
};
