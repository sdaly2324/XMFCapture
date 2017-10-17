#include "PresentationDescriptor.h"
#include "IMFWrapper.h"

#include <mfidl.h>
#include <mfapi.h>

class  PresentationDescriptorRep : public IMFWrapper
{
public:
	PresentationDescriptorRep(CComPtr<IMFMediaSource> mediaSource);
	~PresentationDescriptorRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFPresentationDescriptor>	GetPresentationDescriptor();
	CComPtr<IMFStreamDescriptor>		GetFirstVideoStreamDescriptor();
	CComPtr<IMFStreamDescriptor>		GetFirstAudioStreamDescriptor();

private:
	CComPtr<IMFStreamDescriptor> GetFirstStreamDescriptor(GUID MAJOR_TYPE);
	CComPtr<IMFPresentationDescriptor> mPresentationDescriptor = NULL;
};

PresentationDescriptor::PresentationDescriptor(CComPtr<IMFMediaSource> mediaSource)
{
	m_pRep = std::unique_ptr<PresentationDescriptorRep>(new PresentationDescriptorRep(mediaSource));
}
PresentationDescriptorRep::PresentationDescriptorRep(CComPtr<IMFMediaSource> mediaSource)
{
	OnERR_return(mediaSource->CreatePresentationDescriptor(&mPresentationDescriptor));
}
PresentationDescriptor::~PresentationDescriptor()
{
}
PresentationDescriptorRep::~PresentationDescriptorRep()
{
}

HRESULT PresentationDescriptor::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT PresentationDescriptorRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFPresentationDescriptor> PresentationDescriptor::GetPresentationDescriptor()
{
	return m_pRep->GetPresentationDescriptor();
}
CComPtr<IMFPresentationDescriptor> PresentationDescriptorRep::GetPresentationDescriptor()
{
	return mPresentationDescriptor;
}

CComPtr<IMFStreamDescriptor> PresentationDescriptor::GetFirstVideoStreamDescriptor()
{
	return m_pRep->GetFirstVideoStreamDescriptor();
}
CComPtr<IMFStreamDescriptor> PresentationDescriptorRep::GetFirstVideoStreamDescriptor()
{
	return GetFirstStreamDescriptor(MFMediaType_Video);
}
CComPtr<IMFStreamDescriptor> PresentationDescriptor::GetFirstAudioStreamDescriptor()
{
	return m_pRep->GetFirstAudioStreamDescriptor();
}
CComPtr<IMFStreamDescriptor> PresentationDescriptorRep::GetFirstAudioStreamDescriptor()
{
	return GetFirstStreamDescriptor(MFMediaType_Audio);
}
CComPtr<IMFStreamDescriptor> PresentationDescriptorRep::GetFirstStreamDescriptor(GUID MAJOR_TYPE)
{
	unsigned int streams = 0;
	OnERR_return_NULL(mPresentationDescriptor->GetCount(&streams));
	if (streams == 0)
	{
		streams = 1; // I have no idea why GetCount returns 0 when there is only 1 stream
	}
	for (unsigned int streamID = 0; streamID < streams && LastHR_OK(); streamID++)
	{
		BOOL streamSelected = FALSE;
		CComPtr<IMFStreamDescriptor> currentStreamDescriptor = NULL;
		OnERR_return_NULL(mPresentationDescriptor->GetStreamDescriptorByIndex(streamID, &streamSelected, &currentStreamDescriptor));
		CComPtr<IMFMediaTypeHandler> mediaTypeHandler = NULL;
		OnERR_return_NULL(currentStreamDescriptor->GetMediaTypeHandler(&mediaTypeHandler));
		unsigned long mediaTypes = 0;
		OnERR_return_NULL(mediaTypeHandler->GetMediaTypeCount(&mediaTypes));
		for (unsigned int mediaType = 0; (mediaType < mediaTypes) && LastHR_OK(); mediaType++)
		{
			CComPtr<IMFMediaType> mediaTypeAPI = NULL;
			OnERR_return_NULL(mediaTypeHandler->GetMediaTypeByIndex(mediaType, &mediaTypeAPI));
			GUID guidValue = GUID_NULL;
			OnERR_return_NULL(mediaTypeAPI->GetGUID(MF_MT_MAJOR_TYPE, &guidValue));
			if (guidValue == MAJOR_TYPE)
			{
				return currentStreamDescriptor;
			}
		}
	}
	return NULL;
}