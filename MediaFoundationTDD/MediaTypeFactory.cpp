#include "MediaTypeFactory.h"
#include "MFUtils.h"
#include "AttributesFactory.h"

#include <mfapi.h>
#include <mfidl.h>

class MediaTypeFactoryRep : public MFUtils
{
public:
	MediaTypeFactoryRep();
	virtual ~MediaTypeFactoryRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaType>				CreateAudioEncodingMediaType();
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
		// 43 is 
		//MF_MT_AUDIO_SAMPLES_PER_SECOND	48000
		//MF_MT_AUDIO_NUM_CHANNELS			2
		//MF_MT_AVG_BITRATE					192000
		
		OnERR_return_NULL(GetCollectionObject(availableTypes, 43, &retVal)); // HARD CODED to format 43!!
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
	CComPtr<IMFAttributes> outAttrs = attributesFactory.CreateVideoOutAttrs(inAttrs);
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