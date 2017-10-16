#include "MediaSource.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <mfidl.h>

class MediaSourceRep : public IMFWrapper
{
public:
	MediaSourceRep(CComPtr<IMFActivate> singleDevice);
	MediaSourceRep(CComPtr<IMFActivate> videoDevice, CComPtr<IMFActivate> audioDevice);
	~MediaSourceRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaSource>				GetMediaSource();

private:
	CComPtr<IMFMediaSource>		CreateMediaSource(CComPtr<IMFActivate> singleDevice);
	MediaSourceRep();
	CComPtr<IMFMediaSource>		mMediaSource = NULL;
};

MediaSource::MediaSource(CComPtr<IMFActivate> singleDevice)
{
	m_pRep = std::unique_ptr<MediaSourceRep>(new MediaSourceRep(singleDevice));
}
MediaSourceRep::MediaSourceRep(CComPtr<IMFActivate> singleDevice)
{
	mMediaSource = CreateMediaSource(singleDevice);
}
MediaSource::MediaSource(CComPtr<IMFActivate> videoDevice, CComPtr<IMFActivate> audioDevice)
{
	m_pRep = std::unique_ptr<MediaSourceRep>(new MediaSourceRep(videoDevice, audioDevice));
}
MediaSourceRep::MediaSourceRep(CComPtr<IMFActivate> videoDevice, CComPtr<IMFActivate> audioDevice)
{
	CComPtr<IMFMediaSource>	 videoSource = NULL;
	if (LastHR_OK())
	{
		videoSource = CreateMediaSource(videoDevice);
	}
	CComPtr<IMFMediaSource>	 audioSource = NULL;
	if (LastHR_OK())
	{
		audioSource = CreateMediaSource(audioDevice);
	}
	if (LastHR_OK())
	{
		CComPtr<IMFCollection> collection = NULL;
		PrintIfErrAndSave(MFCreateCollection(&collection));
		if (LastHR_OK())
		{
			PrintIfErrAndSave(collection->AddElement(videoSource));
		}
		if (LastHR_OK())
		{
			PrintIfErrAndSave(collection->AddElement(audioSource));
		}
		if (LastHR_OK())
		{
			PrintIfErrAndSave(MFCreateAggregateSource(collection, &mMediaSource));
		}
	}
}
MediaSource::~MediaSource()
{
}
MediaSourceRep::~MediaSourceRep()
{
}

HRESULT MediaSource::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT MediaSourceRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFMediaSource>	MediaSourceRep::CreateMediaSource(CComPtr<IMFActivate> singleDevice)
{
	CComPtr<IMFMediaSource> retVal = NULL;
	PrintIfErrAndSave(singleDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&retVal));
	if (!LastHR_OK() || !retVal)
	{
		return NULL;
	}
	return retVal;
}

CComPtr<IMFMediaSource> MediaSource::GetMediaSource()
{
	return m_pRep->GetMediaSource();
}
CComPtr<IMFMediaSource> MediaSourceRep::GetMediaSource()
{
	return mMediaSource;
}