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

	HRESULT						GetLastHRESULT();

	void						CreateMediaSession();
	CComPtr<IMFMediaSession>	GetMediaSession();

	void						CreateTopology();
	CComPtr<IMFTopology>		GetTopology();

	void						CreateMediaSource(CComPtr<IMFActivate> myDevice);
	CComPtr<IMFMediaSource>		GetMediaSource();

	CComPtr<IMFActivate>		CreateVideoDevice(std::wstring videoDeviceName);
	CComPtr<IMFActivate>		CreateAudioDevice(std::wstring audioDeviceName);

private:
	Devices*					CreateDevicesFromAttributes(CComPtr<IMFAttributes> attributes);

	AttributesFactory*			mAttributesFactory = NULL;

	CComPtr<IMFMediaSession>	mMediaSessionPtr	= NULL;
	CComPtr<IMFTopology>		mTopologyPtr		= NULL;

	
	CComPtr<IMFMediaSource>		mMediaSourcePtr		= NULL;
	CComPtr<IMFSourceReader>	mSourceReaderPtr	= NULL;
};

MediaFoundationTDD::MediaFoundationTDD()
{
	m_pRep = new MediaFoundationTDDRep();
}
MediaFoundationTDDRep::MediaFoundationTDDRep()
{
	MFStartup(MF_VERSION);
	mAttributesFactory = new AttributesFactory();
}
MediaFoundationTDD::~MediaFoundationTDD()
{
	delete m_pRep;
}
MediaFoundationTDDRep::~MediaFoundationTDDRep()
{
	delete mAttributesFactory;
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
CComPtr<IMFMediaSession> MediaFoundationTDD::GetMediaSession()
{
	return m_pRep->GetMediaSession();
}
CComPtr<IMFMediaSession> MediaFoundationTDDRep::GetMediaSession()
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
CComPtr<IMFTopology> MediaFoundationTDD::GetTopology()
{
	return m_pRep->GetTopology();
}
CComPtr<IMFTopology> MediaFoundationTDDRep::GetTopology()
{
	return mTopologyPtr;
}

void MediaFoundationTDD::CreateMediaSource(CComPtr<IMFActivate> myDevice)
{
	return m_pRep->CreateMediaSource(myDevice);
}
void MediaFoundationTDDRep::CreateMediaSource(CComPtr<IMFActivate> myDevice)
{
	PrintIfErrAndSave(myDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&mMediaSourcePtr));
}
CComPtr<IMFMediaSource> MediaFoundationTDD::GetMediaSource()
{
	return m_pRep->GetMediaSource();
}
CComPtr<IMFMediaSource> MediaFoundationTDDRep::GetMediaSource()
{
	return mMediaSourcePtr;
}

Devices* MediaFoundationTDDRep::CreateDevicesFromAttributes(CComPtr<IMFAttributes> attributes)
{
	Devices* devices = new Devices(attributes);
	if (!devices ||
		devices->GetLastHRESULT() != S_OK ||
		devices->GetDeviceNames().size() <= 0)
	{
		delete devices;
		return NULL;
	}
	return devices;
}

CComPtr<IMFActivate> MediaFoundationTDD::CreateVideoDevice(std::wstring videoDeviceName)
{
	return m_pRep->CreateVideoDevice(videoDeviceName);
}
CComPtr<IMFActivate> MediaFoundationTDDRep::CreateVideoDevice(std::wstring videoDeviceName)
{
	CComPtr<IMFAttributes> myVideoDeviceAttributes = mAttributesFactory->CreateVideoDeviceAttributes();
	if (!myVideoDeviceAttributes || mAttributesFactory->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}

	Devices* myVideoDevices = CreateDevicesFromAttributes(myVideoDeviceAttributes);
	if (!myVideoDevices)
	{
		delete myVideoDevices;
		return NULL;
	}

	CComPtr<IMFActivate> retVal = myVideoDevices->GetDeviceByName(videoDeviceName);

	delete myVideoDevices;
	return retVal;
}

CComPtr<IMFActivate> MediaFoundationTDD::CreateAudioDevice(std::wstring audioDeviceName)
{
	return m_pRep->CreateAudioDevice(audioDeviceName);
}
CComPtr<IMFActivate> MediaFoundationTDDRep::CreateAudioDevice(std::wstring audioDeviceName)
{
	CComPtr<IMFAttributes> myAudioDeviceAttributes = mAttributesFactory->CreateAudioDeviceAttributes();
	if (!myAudioDeviceAttributes || mAttributesFactory->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}

	Devices* myAudioDevices = CreateDevicesFromAttributes(myAudioDeviceAttributes);
	if (!myAudioDevices)
	{
		delete myAudioDevices;
		return NULL;
	}

	CComPtr<IMFActivate> retVal = myAudioDevices->GetDeviceByName(audioDeviceName);
	delete myAudioDevices;
	return retVal;
}