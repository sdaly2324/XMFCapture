#include "MediaTypeFactory.h"
#include "MFUtils.h"
#include "AttributesFactory.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <evr.h>
#include <d3d9.h>
#include <Dxva2api.h>

class MediaTypeFactoryRep : public MFUtils
{
public:
	MediaTypeFactoryRep();
	virtual ~MediaTypeFactoryRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaType>				CreateAudioEncodingMediaType();
	CComPtr<IMFMediaType>				CreateAudioInputMediaType();
	CComPtr<IMFMediaType>				CreateVideoEncodingMediaType(CComPtr<IMFAttributes> inAttrs);

private:
	template <class T>
	HRESULT GetCollectionObject(CComPtr<IMFCollection> collection, DWORD index, T **object)
	{
		CComPtr<IUnknown> unknown;
		HRESULT hr = collection->GetElement(index, &unknown);
		if (SUCCEEDED(hr))
		{
			hr = unknown->QueryInterface(IID_PPV_ARGS(object));
		}
		return hr;
	}
	void DumpAvailableAACFormats(CComPtr<IMFCollection> availableTypes);
};


MediaTypeFactory::MediaTypeFactory()
{
	m_pRep = std::unique_ptr<MediaTypeFactoryRep>(new MediaTypeFactoryRep());
}
MediaTypeFactoryRep::MediaTypeFactoryRep()
{

}
MediaTypeFactory::~MediaTypeFactory()
{
}
MediaTypeFactoryRep::~MediaTypeFactoryRep()
{
}

HRESULT MediaTypeFactory::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT MediaTypeFactoryRep::GetLastHRESULT()
{
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFMediaType> MediaTypeFactory::CreateAudioInputMediaType()
{
	return m_pRep->CreateAudioInputMediaType();
}
CComPtr<IMFMediaType> MediaTypeFactoryRep::CreateAudioInputMediaType()
{
	CComPtr<IMFAttributes> attributes = NULL;
	OnERR_return_NULL(MFCreateAttributes(&attributes, 0));
	OnERR_return_NULL(attributes->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
	OnERR_return_NULL(attributes->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
	OnERR_return_NULL(attributes->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16));
	OnERR_return_NULL(attributes->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 48000));
	OnERR_return_NULL(attributes->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2));

	CComPtr<IMFMediaType> retVal = NULL;
	OnERR_return_NULL(MFCreateMediaType(&retVal));
	OnERR_return_NULL(attributes->CopyAllItems(retVal));
	return retVal;
}

CComPtr<IMFMediaType> MediaTypeFactory::CreateAudioEncodingMediaType()
{
	return m_pRep->CreateAudioEncodingMediaType();
}
CComPtr<IMFMediaType> MediaTypeFactoryRep::CreateAudioEncodingMediaType()
{
	CComPtr<IMFMediaType> retVal = NULL;
	AttributesFactory attributesFactory;
	CComPtr<IMFAttributes> attributes = attributesFactory.CreateAudioOutAttrs();
	if (attributes)
	{
		CComPtr<IMFCollection> availableTypes = NULL;
		OnERR_return_NULL(MFTranscodeGetAudioOutputAvailableTypes(MFAudioFormat_AAC, MFT_ENUM_FLAG_ALL | MFT_ENUM_FLAG_SORTANDFILTER, attributes, &availableTypes));
		
		//DumpAvailableAACFormats(availableTypes);
		DWORD formatIndexWeWantForOutput = 19;
		//	Audio AAC Output(19) ATTR 000 MF_MT_AUDIO_AVG_BYTES_PER_SECOND                         24000
		//	Audio AAC Output(19) ATTR 001 MF_MT_AVG_BITRATE                                        192000
		//	Audio AAC Output(19) ATTR 002 MF_MT_AUDIO_BLOCK_ALIGNMENT                              1
		//	Audio AAC Output(19) ATTR 003 MF_MT_AUDIO_NUM_CHANNELS                                 2
		//	Audio AAC Output(19) ATTR 004 MF_MT_COMPRESSED                                         1
		//	Audio AAC Output(19) ATTR 005 MF_MT_MAJOR_TYPE                                         MFMediaType_Audio
		//	Audio AAC Output(19) ATTR 006 MF_MT_AUDIO_SAMPLES_PER_SECOND                           48000
		//	Audio AAC Output(19) ATTR 007 MF_MT_AM_FORMAT_TYPE                                     FORMAT_WaveFormatEx
		//	Audio AAC Output(19) ATTR 008 MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION                 41
		//	Audio AAC Output(19) ATTR 009 MF_MT_AUDIO_PREFER_WAVEFORMATEX                          1
		//	Audio AAC Output(19) ATTR 010 MF_MT_USER_DATA                                          BLOB
		//	Audio AAC Output(19) ATTR 011 MF_MT_FIXED_SIZE_SAMPLES                                 0
		//	Audio AAC Output(19) ATTR 012 MF_MT_AAC_PAYLOAD_TYPE                                   0
		//	Audio AAC Output(19) ATTR 013 MF_MT_AUDIO_BITS_PER_SAMPLE                              16
		//	Audio AAC Output(19) ATTR 014 MF_MT_SUBTYPE                                            MFAudioFormat_AAC

		OnERR_return_NULL(GetCollectionObject(availableTypes, formatIndexWeWantForOutput, &retVal));
	}
	return retVal;
}
void MediaTypeFactoryRep::DumpAvailableAACFormats(CComPtr<IMFCollection> availableTypes)
{
	// DUMP available AAC formats
	DWORD count = 0;
	HRESULT hr = availableTypes->GetElementCount(&count);
	for (DWORD i = 0; i < count; i++)
	{
		CComPtr<IMFMediaType> pMediaType = NULL;
		hr = GetCollectionObject(availableTypes, i, &pMediaType);
		WCHAR count[1024];
		swprintf_s(count, 1024, L"%d", i);
		DumpAttr(pMediaType, L"AUDIO AAC", count);
		OutputDebugStringW(L"\n");
	}
}

CComPtr<IMFMediaType> MediaTypeFactory::CreateVideoEncodingMediaType(CComPtr<IMFAttributes> inAttrs)
{
	return m_pRep->CreateVideoEncodingMediaType(inAttrs);
}
CComPtr<IMFMediaType> MediaTypeFactoryRep::CreateVideoEncodingMediaType(CComPtr<IMFAttributes> inAttrs)
{
	AttributesFactory attributesFactory;
	CComPtr<IMFAttributes> outAttrs = attributesFactory.CreateVideoEncodeAttrs(inAttrs);
	if (!outAttrs)
	{
		SetLastHR_Fail();
		return NULL;
	}
	CComPtr<IMFMediaType> retVal = NULL;
	OnERR_return_NULL(MFCreateMediaType(&retVal));
	OnERR_return_NULL(outAttrs->CopyAllItems(retVal));
	return retVal;
}