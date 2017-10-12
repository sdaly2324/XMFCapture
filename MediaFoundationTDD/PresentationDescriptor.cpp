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
	m_pRep = new PresentationDescriptorRep(mediaSource);
}
PresentationDescriptorRep::PresentationDescriptorRep(CComPtr<IMFMediaSource> mediaSource)
{
	PrintIfErrAndSave(mediaSource->CreatePresentationDescriptor(&mPresentationDescriptor));
}
PresentationDescriptor::~PresentationDescriptor()
{
	delete m_pRep;
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
	PrintIfErrAndSave(mPresentationDescriptor->GetCount(&streams));
	if (streams == 0)
	{
		streams = 1; // no idea why it returns 0 when there is only one stream
	}
	for (unsigned int streamID = 0; streamID < streams && LastHR_OK(); streamID++)
	{
		BOOL streamSelected = FALSE;
		CComPtr<IMFStreamDescriptor> currentStreamDescriptor = NULL;
		PrintIfErrAndSave(mPresentationDescriptor->GetStreamDescriptorByIndex(streamID, &streamSelected, &currentStreamDescriptor));
		if (!LastHR_OK() || !currentStreamDescriptor)
		{
			return NULL;
		}
		CComPtr<IMFMediaTypeHandler> mediaTypeHandler = NULL;
		PrintIfErrAndSave(currentStreamDescriptor->GetMediaTypeHandler(&mediaTypeHandler));
		if (!LastHR_OK() || !mediaTypeHandler)
		{
			return NULL;
		}
		unsigned long mediaTypes = 0;
		PrintIfErrAndSave(mediaTypeHandler->GetMediaTypeCount(&mediaTypes));
		if (!LastHR_OK() || mediaTypes <= 0)
		{
			return NULL;
		}
		for (unsigned int mediaType = 0; (mediaType < mediaTypes) && LastHR_OK(); mediaType++)
		{
			CComPtr<IMFMediaType> mediaTypeAPI = NULL;
			PrintIfErrAndSave(mediaTypeHandler->GetMediaTypeByIndex(mediaType, &mediaTypeAPI));
			if (!LastHR_OK() || !mediaTypeAPI)
			{
				return NULL;
			}
			GUID guidValue = GUID_NULL;
			PrintIfErrAndSave(mediaTypeAPI->GetGUID(MF_MT_MAJOR_TYPE, &guidValue));
			if (!LastHR_OK())
			{
				return NULL;
			}
			if (guidValue == MAJOR_TYPE)
			{
				return currentStreamDescriptor;
			}
		}
	}
	return NULL;
}