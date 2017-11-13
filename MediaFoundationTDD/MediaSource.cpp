#include "MediaSource.h"
#include "MFUtils.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class MediaSourceRep : public MFUtils
{
public:
	MediaSourceRep(CComPtr<IMFActivate> singleDevice);
	MediaSourceRep(CComPtr<IMFActivate> videoDevice, CComPtr<IMFActivate> audioDevice);
	~MediaSourceRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaSource>				GetMediaSource();
	CComPtr<IMFMediaType>				GetVideoMediaType();
	CComPtr<IMFMediaType>				GetAudioMediaType();
	void								SetCurrentMediaTypes();

private:
	void							SetCurrentVideoMediaType();
	void							SetCurrentAudioMediaType();
	CComPtr<IMFMediaTypeHandler>	GetMediaTypeHandler(GUID mediaTypeGUIDWeWant);
	void							SetVideoMediaTypeHandler();
	void							SetAudioMediaTypeHandler();
	CComPtr<IMFMediaSource>			CreateMediaSource(CComPtr<IMFActivate> singleDevice);

	CComPtr<IMFMediaSource>			mMediaSource = nullptr;
	CComPtr<IMFActivate>			mVideoDevice = nullptr;
	CComPtr<IMFActivate>			mAudioDevice = nullptr;
	CComPtr<IMFMediaTypeHandler>	mVideoMediaTypeHandler = nullptr;
	CComPtr<IMFMediaTypeHandler>	mAudioMediaTypeHandler = nullptr;
};

MediaSource::MediaSource(CComPtr<IMFActivate> singleDevice)
{
	m_pRep = std::unique_ptr<MediaSourceRep>(new MediaSourceRep(singleDevice));
}
MediaSourceRep::MediaSourceRep(CComPtr<IMFActivate> singleDevice)
{
	mMediaSource = CreateMediaSource(singleDevice);
	SetAudioMediaTypeHandler();
	SetVideoMediaTypeHandler();
}
MediaSource::MediaSource(CComPtr<IMFActivate> videoDevice, CComPtr<IMFActivate> audioDevice)
{
	m_pRep = std::unique_ptr<MediaSourceRep>(new MediaSourceRep(videoDevice, audioDevice));
}
MediaSourceRep::MediaSourceRep(CComPtr<IMFActivate> videoDevice, CComPtr<IMFActivate> audioDevice)
{
	CComPtr<IMFMediaSource>	 videoSource = nullptr;
	videoSource = CreateMediaSource(videoDevice);
	CComPtr<IMFMediaSource>	 audioSource = nullptr;
	if (LastHR_OK())
	{
		audioSource = CreateMediaSource(audioDevice);
	}
	if (LastHR_OK())
	{
		CComPtr<IMFCollection> collection = nullptr;
		OnERR_return(MFCreateCollection(&collection));
		OnERR_return(collection->AddElement(videoSource));
		OnERR_return(collection->AddElement(audioSource));
		OnERR_return(MFCreateAggregateSource(collection, &mMediaSource));
	}
	SetAudioMediaTypeHandler();
	SetVideoMediaTypeHandler();
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
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFMediaSource>	MediaSourceRep::CreateMediaSource(CComPtr<IMFActivate> singleDevice)
{
	CComPtr<IMFMediaSource> retVal = nullptr;
	OnERR_return_NULL(singleDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&retVal));
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

CComPtr<IMFMediaType> MediaSource::GetVideoMediaType()
{
	return m_pRep->GetVideoMediaType();
}
CComPtr<IMFMediaType> MediaSourceRep::GetVideoMediaType()
{
	if (!mVideoMediaTypeHandler)
	{
		return nullptr;
	}
	CComPtr<IMFMediaType> currentMediaType = nullptr;
	OnERR_return_NULL(mVideoMediaTypeHandler->GetCurrentMediaType(&currentMediaType));
	return currentMediaType;
}

CComPtr<IMFMediaType> MediaSource::GetAudioMediaType()
{
	return m_pRep->GetAudioMediaType();
}
CComPtr<IMFMediaType> MediaSourceRep::GetAudioMediaType()
{
	if (!mAudioMediaTypeHandler)
	{
		return nullptr;
	}
	CComPtr<IMFMediaType> currentMediaType = nullptr;
	OnERR_return_NULL(mAudioMediaTypeHandler->GetCurrentMediaType(&currentMediaType));
	return currentMediaType;
}

CComPtr<IMFMediaTypeHandler> MediaSourceRep::GetMediaTypeHandler(GUID mediaTypeGUIDWeWant)
{
	CComPtr<IMFMediaTypeHandler> retVal = nullptr;

	CComPtr<IMFPresentationDescriptor> presentationDescriptor = nullptr;
	OnERR_return_NULL(mMediaSource->CreatePresentationDescriptor(&presentationDescriptor));

	DWORD streamDescriptorCount = 0;
	OnERR_return_NULL(presentationDescriptor->GetStreamDescriptorCount(&streamDescriptorCount));
	for (DWORD streamDescriptorIndex = 0; streamDescriptorIndex < streamDescriptorCount; streamDescriptorIndex++)
	{
		BOOL selected = FALSE;
		CComPtr<IMFStreamDescriptor> currentStreamDescriptor = nullptr;
		OnERR_return_NULL(presentationDescriptor->GetStreamDescriptorByIndex(streamDescriptorIndex, &selected, &currentStreamDescriptor));
		retVal = nullptr;
		OnERR_return_NULL(currentStreamDescriptor->GetMediaTypeHandler(&retVal));
		GUID currentMediaTypeGUID = GUID_NULL;
		OnERR_return_NULL(retVal->GetMajorType(&currentMediaTypeGUID));
		if (currentMediaTypeGUID == mediaTypeGUIDWeWant)
		{
			return retVal;
		}
	}
	return nullptr;
}
void MediaSourceRep::SetAudioMediaTypeHandler()
{
	mAudioMediaTypeHandler = GetMediaTypeHandler(MFMediaType_Audio);
}
void MediaSourceRep::SetVideoMediaTypeHandler()
{
	mVideoMediaTypeHandler = GetMediaTypeHandler(MFMediaType_Video);
}

void MediaSource::SetCurrentMediaTypes()
{
	m_pRep->SetCurrentMediaTypes();
}
void MediaSourceRep::SetCurrentMediaTypes()
{
	SetCurrentVideoMediaType();
	SetCurrentAudioMediaType();
}

void MediaSourceRep::SetCurrentVideoMediaType()
{
	if (!mVideoMediaTypeHandler)
	{
		return;
	}
	CComPtr<IMFMediaType> mediaType = nullptr;
	DWORD formatWeWant = 64;	// MXL is 64 for 720p 5994 YUY2
	OnERR_return(mVideoMediaTypeHandler->GetMediaTypeByIndex(formatWeWant, &mediaType));
	OnERR_return(mVideoMediaTypeHandler->SetCurrentMediaType(mediaType));
}
void MediaSourceRep::SetCurrentAudioMediaType()
{
	if (!mAudioMediaTypeHandler)
	{
		return;
	}
	CComPtr<IMFMediaType> mediaType = nullptr;
	DWORD formatWeWant = 0;		// 2 channels 48k 16 bit PCM
	DWORD count = 0;
	OnERR_return(mAudioMediaTypeHandler->GetMediaTypeByIndex(formatWeWant, &mediaType));
	OnERR_return(mAudioMediaTypeHandler->SetCurrentMediaType(mediaType));
}