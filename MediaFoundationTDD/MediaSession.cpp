#include "MediaSession.h"
#include "IMFWrapper.h"

#include <mfidl.h>

class MediaSessionRep : public IMFWrapper
{
public:
	MediaSessionRep();
	~MediaSessionRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaSession>			GetMediaSession();

private:
	CComPtr<IMFMediaSession>			mMediaSession = NULL;
};

MediaSession::MediaSession()
{
	m_pRep = new MediaSessionRep();
}
MediaSessionRep::MediaSessionRep()
{
	PrintIfErrAndSave(MFCreateMediaSession(NULL, &mMediaSession));
}
MediaSession::~MediaSession()
{
	delete m_pRep;
}
MediaSessionRep::~MediaSessionRep()
{
}

HRESULT MediaSession::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT MediaSessionRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFMediaSession> MediaSession::GetMediaSession()
{
	return m_pRep->GetMediaSession();
}
CComPtr<IMFMediaSession> MediaSessionRep::GetMediaSession()
{
	return mMediaSession;
}