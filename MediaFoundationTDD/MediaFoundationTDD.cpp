#include "MediaFoundationTDD.h"
#include "IMFWrapper.h"
#include "AttributesFactory.h"
#include "Devices.h"

#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <vector>

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

	IMFActivate*		CreateVideoOnlyDevice(std::wstring videoDeviceName);

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

IMFActivate* MediaFoundationTDD::CreateVideoOnlyDevice(std::wstring videoDeviceName)
{
	return m_pRep->CreateVideoOnlyDevice(videoDeviceName);
}
IMFActivate* MediaFoundationTDDRep::CreateVideoOnlyDevice(std::wstring videoDeviceName)
{
	IMFActivate* retVal = NULL;
	AttributesFactory* myAttributesFactory = new AttributesFactory();
	IMFAttributes* myVideoDeviceAttributes = myAttributesFactory->CreateVideoDeviceAttributes();
	if (!myVideoDeviceAttributes || myAttributesFactory->GetLastHRESULT() != S_OK)
	{
		delete myAttributesFactory;
		return NULL;
	}

	Devices* myVideoDevices = new Devices(myVideoDeviceAttributes);
	if (!myVideoDevices ||
		myVideoDevices->GetLastHRESULT() != S_OK ||
		myVideoDevices->GetNumDevices() <= 0)
	{
		delete myVideoDevices;
		delete myAttributesFactory;
		return NULL;
	}

	std::vector<std::wstring> myVideoDeviceNames = myVideoDevices->GetDeviceNames();
	bool foundVideoDevice = false;
	if (std::find(myVideoDeviceNames.begin(), myVideoDeviceNames.end(), videoDeviceName) != myVideoDeviceNames.end())
	{
		foundVideoDevice = true;
	}
	if (foundVideoDevice)
	{
		retVal = myVideoDevices->GetDeviceByName(videoDeviceName);
	}
	delete myVideoDevices;
	delete myAttributesFactory;
	return retVal;
}