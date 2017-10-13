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

	void						CreateTopology();
	CComPtr<IMFTopology>		GetTopology();

	CComPtr<IMFActivate>		CreateVideoDevice(std::wstring videoDeviceName);
	CComPtr<IMFActivate>		CreateAudioDevice(std::wstring audioDeviceName);

	CComPtr<IMFMediaSource>		CreateMediaSource(CComPtr<IMFActivate> myDevice);
	CComPtr<IMFMediaSource>		CreateAggregateMediaSource(CComPtr<IMFMediaSource> pVideoSource, CComPtr<IMFMediaSource> pAudioSource);

private:

	AttributesFactory*			mAttributesFactory = NULL;
	CComPtr<IMFTopology>		mTopologyPtr		= NULL;
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
}

HRESULT MediaFoundationTDD::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT MediaFoundationTDDRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
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

CComPtr<IMFMediaSource>	 MediaFoundationTDD::CreateMediaSource(CComPtr<IMFActivate> myDevice)
{
	return m_pRep->CreateMediaSource(myDevice);
}
CComPtr<IMFMediaSource>	 MediaFoundationTDDRep::CreateMediaSource(CComPtr<IMFActivate> myDevice)
{
	CComPtr<IMFMediaSource>	retVal = NULL;
	PrintIfErrAndSave(myDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&retVal));
	if (!LastHR_OK() || !retVal)
	{
		return NULL;
	}
	return retVal;
}

CComPtr<IMFMediaSource> MediaFoundationTDD::CreateAggregateMediaSource(CComPtr<IMFMediaSource> pVideoSource, CComPtr<IMFMediaSource> pAudioSource)
{
	return m_pRep->CreateAggregateMediaSource(pVideoSource, pAudioSource);
}
CComPtr<IMFMediaSource> MediaFoundationTDDRep::CreateAggregateMediaSource(CComPtr<IMFMediaSource> pVideoSource, CComPtr<IMFMediaSource> pAudioSource)
{
	CComPtr<IMFCollection> collection = NULL;
	PrintIfErrAndSave(MFCreateCollection(&collection));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(collection->AddElement(pAudioSource));
	}
	if (LastHR_OK())
	{
		PrintIfErrAndSave(collection->AddElement(pVideoSource));
	}
	CComPtr<IMFMediaSource> AVMediaSource = NULL;
	if (LastHR_OK())
	{
		PrintIfErrAndSave(MFCreateAggregateSource(collection, &AVMediaSource));
	}
	if (LastHR_OK() && AVMediaSource)
	{
		return AVMediaSource;
	}
	return NULL;
}