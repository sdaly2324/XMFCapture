#pragma once
#include <windows.h>
#include <atlcomcli.h>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSession;
class MediaSessionRep;
class MediaFoundationTDD_API MediaSession
{
public:
	MediaSession();
	~MediaSession();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaSession>			GetMediaSession();

private:
	MediaSessionRep* m_pRep = 0;
};
