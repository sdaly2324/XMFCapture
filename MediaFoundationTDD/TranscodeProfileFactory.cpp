#include "TranscodeProfileFactory.h"
#include "MFUtils.h"
#include "AttributesFactory.h"
#include "MediaTypeFactory.h"

#include <mfapi.h>
#include <mfidl.h>

class TranscodeProfileFactoryRep : public MFUtils
{
public:
	TranscodeProfileFactoryRep();
	virtual ~TranscodeProfileFactoryRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTranscodeProfile>		CreateVideoOnlyTranscodeProfile(CComPtr<IMFAttributes> videoInputAttributes);
	CComPtr<IMFTranscodeProfile>		CreateAudioOnlyTranscodeProfile();
private:
	void SetVideoAttributes(CComPtr<IMFTranscodeProfile> transcodeProfile, CComPtr<IMFAttributes> videoInputAttributes);
	void SetAudioAttributes(CComPtr<IMFTranscodeProfile> transcodeProfile);
	void SetContainerAttributes(CComPtr<IMFTranscodeProfile> transcodeProfile);
};


TranscodeProfileFactory::TranscodeProfileFactory()
{
	m_pRep =  std::unique_ptr<TranscodeProfileFactoryRep>(new TranscodeProfileFactoryRep());
}
TranscodeProfileFactoryRep::TranscodeProfileFactoryRep()
{

}
TranscodeProfileFactory::~TranscodeProfileFactory()
{
}
TranscodeProfileFactoryRep::~TranscodeProfileFactoryRep()
{
}

HRESULT TranscodeProfileFactory::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT TranscodeProfileFactoryRep::GetLastHRESULT()
{
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFTranscodeProfile> TranscodeProfileFactory::CreateVideoOnlyTranscodeProfile(CComPtr<IMFAttributes> videoInputAttributes)
{
	return m_pRep->CreateVideoOnlyTranscodeProfile(videoInputAttributes);
}
CComPtr<IMFTranscodeProfile> TranscodeProfileFactoryRep::CreateVideoOnlyTranscodeProfile(CComPtr<IMFAttributes> videoInputAttributes)
{
	CComPtr<IMFTranscodeProfile> transcodeProfile = NULL;
	OnERR_return_NULL(MFCreateTranscodeProfile(&transcodeProfile));
	SetVideoAttributes(transcodeProfile, videoInputAttributes);
	OnERR_return_NULL(GetLastHRESULT());
	SetContainerAttributes(transcodeProfile);
	OnERR_return_NULL(GetLastHRESULT());
	return transcodeProfile;
}
void TranscodeProfileFactoryRep::SetVideoAttributes(CComPtr<IMFTranscodeProfile> transcodeProfile, CComPtr<IMFAttributes> videoInputAttributes)
{
	MediaTypeFactory mediaTypeFactory;
	CComPtr<IMFAttributes> videoOutAttrs = mediaTypeFactory.CreateVideoEncodingMediaType(videoInputAttributes);

	OnERR_return(mediaTypeFactory.GetLastHRESULT());
	OnERR_return(transcodeProfile->SetVideoAttributes(videoOutAttrs));
}
void TranscodeProfileFactoryRep::SetAudioAttributes(CComPtr<IMFTranscodeProfile> transcodeProfile)
{
	MediaTypeFactory mediaTypeFactory;
	CComPtr<IMFAttributes> audioOutAttrs = mediaTypeFactory.CreateAudioEncodingMediaType();
	OnERR_return(mediaTypeFactory.GetLastHRESULT());
	OnERR_return(transcodeProfile->SetAudioAttributes(audioOutAttrs));
}
void TranscodeProfileFactoryRep::SetContainerAttributes(CComPtr<IMFTranscodeProfile> transcodeProfile)
{
	AttributesFactory attributesFactory;
	CComPtr<IMFAttributes> fileSinkAttrs = attributesFactory.CreateFileSinkAttrs();
	OnERR_return(attributesFactory.GetLastHRESULT());
	OnERR_return(transcodeProfile->SetContainerAttributes(fileSinkAttrs));
}

CComPtr<IMFTranscodeProfile> TranscodeProfileFactory::CreateAudioOnlyTranscodeProfile()
{
	return m_pRep->CreateAudioOnlyTranscodeProfile();
}
CComPtr<IMFTranscodeProfile> TranscodeProfileFactoryRep::CreateAudioOnlyTranscodeProfile()
{
	CComPtr<IMFTranscodeProfile> transcodeProfile = NULL;
	OnERR_return_NULL(MFCreateTranscodeProfile(&transcodeProfile));
	SetAudioAttributes(transcodeProfile);
	OnERR_return_NULL(GetLastHRESULT());
	SetContainerAttributes(transcodeProfile);
	OnERR_return_NULL(GetLastHRESULT());
	return transcodeProfile;
}