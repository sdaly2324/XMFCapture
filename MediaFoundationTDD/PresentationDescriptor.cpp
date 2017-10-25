#include "PresentationDescriptor.h"
#include "MFUtils.h"

#include <mfidl.h>
#include <mfapi.h>

class  PresentationDescriptorRep : public MFUtils
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
	return MFUtils::GetLastHRESULT();
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
	DWORD streamCount = 0;
	OnERR_return_NULL(mPresentationDescriptor->GetStreamDescriptorCount(&streamCount));
	for (unsigned int streamID = 0; streamID < streamCount && LastHR_OK(); streamID++)
	{
		BOOL streamSelected = FALSE;
		CComPtr<IMFStreamDescriptor> currentStreamDescriptor = NULL;
		OnERR_return_NULL(mPresentationDescriptor->GetStreamDescriptorByIndex(streamID, &streamSelected, &currentStreamDescriptor));
		CComPtr<IMFMediaTypeHandler> mediaTypeHandler = NULL;
		OnERR_return_NULL(currentStreamDescriptor->GetMediaTypeHandler(&mediaTypeHandler));
		unsigned long mediaTypes = 0;
		OnERR_return_NULL(mediaTypeHandler->GetMediaTypeCount(&mediaTypes));
		for (unsigned int mediaTypeIndex = 0; (mediaTypeIndex < mediaTypes) && LastHR_OK(); mediaTypeIndex++)
		{
			CComPtr<IMFMediaType> mediaType = NULL;
			OnERR_return_NULL(mediaTypeHandler->GetMediaTypeByIndex(mediaTypeIndex, &mediaType));
			GUID guidValue = GUID_NULL;
			OnERR_return_NULL(mediaType->GetGUID(MF_MT_MAJOR_TYPE, &guidValue));
			if (guidValue == MAJOR_TYPE)
			{
				//DumpAttr(mediaType, L"GetFirstStreamDescriptor matching Stream " + std::to_wstring(streamID) + L" of " + std::to_wstring(streamCount), L"mediaTypeIndex " + std::to_wstring(mediaTypeIndex) + L" of " + std::to_wstring(mediaTypes));
				return currentStreamDescriptor;
			}
		}
	}
	return NULL;
}