#include "MediaFoundationTDD.h"
#include "IMFWrapper.h"
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>

class MediaFoundationTDDRep : public IMFWrapper
{
public:
	MediaFoundationTDDRep();
	~MediaFoundationTDDRep();

	HRESULT				GetLastHRESULT();

	void				CreateMediaSession();
	IMFMediaSession*	GetMediaSession();

	void				CreateTopology();
	IMFTopology*		GetTopology();

	void				CreateMediaSource(IMFActivate* myDevice);
	IMFMediaSource*		GetMediaSource();

	void				CreateSourceReader(IMFMediaSource* mediaSource, IMFAttributes* sourceReaderAsycCallbackAttributes);
	IMFSourceReader*	GetSourceReader();

private:
	IMFMediaSession*	mMediaSessionPtr	= NULL;
	IMFTopology*		mTopologyPtr		= NULL;

	
	IMFMediaSource*		mMediaSourcePtr		= NULL;
	IMFSourceReader*	mSourceReaderPtr	= NULL;
};

MediaFoundationTDD::MediaFoundationTDD()
{
	m_pRep = new MediaFoundationTDDRep();
}
MediaFoundationTDDRep::MediaFoundationTDDRep()
{
	MFStartup(MF_VERSION);
}
MediaFoundationTDD::~MediaFoundationTDD()
{
	delete m_pRep;
}
MediaFoundationTDDRep::~MediaFoundationTDDRep()
{
	delete mTopologyPtr;
	delete mMediaSessionPtr;
}

HRESULT MediaFoundationTDD::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT MediaFoundationTDDRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}

void MediaFoundationTDD::CreateMediaSession()
{
	return m_pRep->CreateMediaSession();
}
void MediaFoundationTDDRep::CreateMediaSession()
{
	PrintIfErrAndSave(MFCreateMediaSession(NULL, &mMediaSessionPtr));
}
IMFMediaSession* MediaFoundationTDD::GetMediaSession()
{
	return m_pRep->GetMediaSession();
}
IMFMediaSession* MediaFoundationTDDRep::GetMediaSession()
{
	return mMediaSessionPtr;
}

void MediaFoundationTDD::CreateTopology()
{
	return m_pRep->CreateTopology();
}
void MediaFoundationTDDRep::CreateTopology()
{
	PrintIfErrAndSave(MFCreateTopology(&mTopologyPtr));
}
IMFTopology* MediaFoundationTDD::GetTopology()
{
	return m_pRep->GetTopology();
}
IMFTopology* MediaFoundationTDDRep::GetTopology()
{
	return mTopologyPtr;
}

void MediaFoundationTDD::CreateMediaSource(IMFActivate* myDevice)
{
	return m_pRep->CreateMediaSource(myDevice);
}
void MediaFoundationTDDRep::CreateMediaSource(IMFActivate* myDevice)
{
	PrintIfErrAndSave(myDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&mMediaSourcePtr));
}
IMFMediaSource* MediaFoundationTDD::GetMediaSource()
{
	return m_pRep->GetMediaSource();
}
IMFMediaSource* MediaFoundationTDDRep::GetMediaSource()
{
	return mMediaSourcePtr;
}

void MediaFoundationTDD::CreateSourceReader(IMFMediaSource* mediaSource, IMFAttributes* sourceReaderAsycCallbackAttributes)
{
	return m_pRep->CreateSourceReader(mediaSource, sourceReaderAsycCallbackAttributes);
}
void MediaFoundationTDDRep::CreateSourceReader(IMFMediaSource* mediaSource, IMFAttributes* sourceReaderAsycCallbackAttributes)
{
	PrintIfErrAndSave(MFCreateSourceReaderFromMediaSource(mediaSource, sourceReaderAsycCallbackAttributes, &mSourceReaderPtr));
}
IMFSourceReader* MediaFoundationTDD::GetSourceReader()
{
	return m_pRep->GetSourceReader();
}
IMFSourceReader* MediaFoundationTDDRep::GetSourceReader()
{
	return mSourceReaderPtr;
}