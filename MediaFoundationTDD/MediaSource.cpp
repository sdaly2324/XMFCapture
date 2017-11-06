#include "MediaSource.h"
#include "MFUtils.h"
#include "SourceReader.h"

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
	void								SetVideoMediaType(CComPtr<IMFMediaType> mediaType);
	void								FixVideoMediaType();

private:
	void GetVideoMediaTypeHandler();
	CComPtr<IMFMediaSource>		CreateMediaSource(CComPtr<IMFActivate> singleDevice);
	MediaSourceRep();
	CComPtr<IMFMediaSource>		mMediaSource = NULL;
	CComPtr<IMFActivate>		mVideoDevice = NULL;
	CComPtr<IMFActivate>		mAudioDevice = NULL;
	std::unique_ptr<SourceReader>	mSourceReader = NULL;
	CComPtr<IMFMediaTypeHandler> mVideoMediaTypeHandler = NULL;
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
	videoSource = CreateMediaSource(videoDevice);
	CComPtr<IMFMediaSource>	 audioSource = NULL;
	if (LastHR_OK())
	{
		audioSource = CreateMediaSource(audioDevice);
	}
	if (LastHR_OK())
	{
		CComPtr<IMFCollection> collection = NULL;
		OnERR_return(MFCreateCollection(&collection));
		OnERR_return(collection->AddElement(videoSource));
		OnERR_return(collection->AddElement(audioSource));
		OnERR_return(MFCreateAggregateSource(collection, &mMediaSource));
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
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFMediaSource>	MediaSourceRep::CreateMediaSource(CComPtr<IMFActivate> singleDevice)
{
	CComPtr<IMFMediaSource> retVal = NULL;
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

void MediaSource::SetVideoMediaType(CComPtr<IMFMediaType> mediaType)
{
	m_pRep->SetVideoMediaType(mediaType);
}
void MediaSourceRep::SetVideoMediaType(CComPtr<IMFMediaType> mediaType)
{
	if (!mVideoMediaTypeHandler)
	{
		GetVideoMediaTypeHandler();
	}
	OnERR_return(mVideoMediaTypeHandler->SetCurrentMediaType(mediaType));
}

CComPtr<IMFMediaType> MediaSource::GetVideoMediaType()
{
	return m_pRep->GetVideoMediaType();
}
CComPtr<IMFMediaType> MediaSourceRep::GetVideoMediaType()
{
	if (!mVideoMediaTypeHandler)
	{
		GetVideoMediaTypeHandler();
	}
	CComPtr<IMFMediaType> currentMediaType = NULL;
	OnERR_return_NULL(mVideoMediaTypeHandler->GetCurrentMediaType(&currentMediaType));
	return currentMediaType;
}

CComPtr<IMFMediaType> MediaSource::GetAudioMediaType()
{
	return m_pRep->GetAudioMediaType();
}
CComPtr<IMFMediaType> MediaSourceRep::GetAudioMediaType()
{
	std::unique_ptr<SourceReader> sourceReader = std::make_unique<SourceReader>(mMediaSource);
	if (sourceReader)
	{
		return sourceReader->GetAudioMediaType();
	}
	return NULL;
}

void MediaSourceRep::GetVideoMediaTypeHandler()
{
	mVideoMediaTypeHandler = NULL;
	CComPtr<IMFPresentationDescriptor> presentationDescriptor = NULL;
	OnERR_return(mMediaSource->CreatePresentationDescriptor(&presentationDescriptor));
	DWORD descriptorCount = 0;
	OnERR_return(presentationDescriptor->GetStreamDescriptorCount(&descriptorCount));
	if (descriptorCount > 1)
	{
		OutputDebugStringW(L"GetStreamDescriptorCount found more than 1 descriptor\n");
		SetLastHR_Fail();
		return;
	}
	CComPtr<IMFStreamDescriptor> streamDescriptor = NULL;
	BOOL selected = FALSE;
	OnERR_return(presentationDescriptor->GetStreamDescriptorByIndex(0, &selected, &streamDescriptor));
	OnERR_return(streamDescriptor->GetMediaTypeHandler(&mVideoMediaTypeHandler))
}

void MediaSource::FixVideoMediaType()
{
	m_pRep->FixVideoMediaType();
}
void MediaSourceRep::FixVideoMediaType()
{
	if (!mVideoMediaTypeHandler)
	{
		GetVideoMediaTypeHandler();
	}
	CComPtr<IMFMediaType> mediaType = NULL;
	DWORD formatWeWant = 64;	// MXL is 64 for 720p 5994 YUY2
	OnERR_return(mVideoMediaTypeHandler->GetMediaTypeByIndex(formatWeWant, &mediaType));
	OnERR_return(mVideoMediaTypeHandler->SetCurrentMediaType(mediaType));
}