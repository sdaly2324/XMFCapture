#include "AttributesFactory.h"
#include "MFUtils.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class AttributesFactoryRep : public MFUtils
{
public:
	AttributesFactoryRep();
	~AttributesFactoryRep();

	HRESULT GetLastHRESULT();

	CComPtr<IMFAttributes>	CreateVideoDeviceAttrs();
	CComPtr<IMFAttributes>	CreateAudioDeviceAttrs();
	CComPtr<IMFAttributes>	CreateSInkReaderCbAttrs(IUnknown* callBack);
	CComPtr<IMFAttributes>	CreateFileSinkAttrs();
	CComPtr<IMFAttributes>	CreateVideoEncodeAttrs(CComPtr<IMFAttributes> vInAttrs);
	CComPtr<IMFAttributes>	CreateAudioOutAttrs();

private:
	HRESULT					CopyAttribute(CComPtr<IMFAttributes> sourceAttribute, CComPtr<IMFAttributes> destinationAttribute, const GUID& attributeGUID);
};
AttributesFactory::AttributesFactory()
{
	m_pRep = std::unique_ptr<AttributesFactoryRep>(new AttributesFactoryRep());
}
AttributesFactoryRep::AttributesFactoryRep()
{	
}
AttributesFactory::~AttributesFactory()
{
}
AttributesFactoryRep::~AttributesFactoryRep()
{
}

HRESULT AttributesFactory::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT AttributesFactoryRep::GetLastHRESULT()
{
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFAttributes> AttributesFactory::CreateVideoDeviceAttrs()
{
	return m_pRep->CreateVideoDeviceAttrs();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateVideoDeviceAttrs()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 1));
	OnERR_return_NULL(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateAudioDeviceAttrs()
{
	return m_pRep->CreateAudioDeviceAttrs();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateAudioDeviceAttrs()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 1));
	OnERR_return_NULL(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateSInkReaderCbAttrs(IUnknown* callBack)
{
	return m_pRep->CreateSInkReaderCbAttrs(callBack);
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateSInkReaderCbAttrs(IUnknown* callBack)
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 0));
		// for some reason SetUnknown does not count towards the IMFAttributes count
	OnERR_return_NULL(retVal->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callBack));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateFileSinkAttrs()
{
	return m_pRep->CreateFileSinkAttrs();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateFileSinkAttrs()
{
	CComPtr<IMFAttributes> attributes = NULL;
	OnERR_return_NULL(MFCreateAttributes(&attributes, 0));
	//OnERR_return_NULL(attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	//OnERR_return_NULL(attributes->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Stream));
	//OnERR_return_NULL(attributes->SetGUID(MF_MT_SUBTYPE, MFStreamFormat_MPEG2Transport));
	OnERR_return_NULL(attributes->SetGUID(MF_TRANSCODE_CONTAINERTYPE, MFTranscodeContainerType_MPEG2));
	OnERR_return_NULL(attributes->SetUINT32(MF_TRANSCODE_ADJUST_PROFILE, MF_TRANSCODE_ADJUST_PROFILE_DEFAULT));
	return attributes;
}

CComPtr<IMFAttributes> AttributesFactory::CreateAudioOutAttrs()
{
	return m_pRep->CreateAudioOutAttrs();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateAudioOutAttrs()
{
	CComPtr<IMFAttributes> attributes = NULL;
	OnERR_return_NULL(MFCreateAttributes(&attributes, 0));
	OnERR_return_NULL(attributes->SetUINT32(MF_LOW_LATENCY, TRUE));
	return attributes;
}

HRESULT AttributesFactoryRep::CopyAttribute(CComPtr<IMFAttributes> sourceAttribute, CComPtr<IMFAttributes> destinationAttribute, const GUID& attributeGUID)
{
	PROPVARIANT var;
	PropVariantInit(&var);
	HRESULT hr = sourceAttribute->GetItem(attributeGUID, &var);
	if (SUCCEEDED(hr))
	{
		hr = destinationAttribute->SetItem(attributeGUID, var);
		PropVariantClear(&var);
	}
	return hr;
}
CComPtr<IMFAttributes> AttributesFactory::CreateVideoEncodeAttrs(CComPtr<IMFAttributes> vInAttrs)
{
	return m_pRep->CreateVideoEncodeAttrs(vInAttrs);
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateVideoEncodeAttrs(CComPtr<IMFAttributes> vInAttrs)
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 0));
	if (!vInAttrs)
	{
		// assume 720p5994
		OnERR_return_NULL(retVal->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		OnERR_return_NULL(retVal->SetUINT64(MF_MT_FRAME_SIZE, 5497558139600));
		OnERR_return_NULL(retVal->SetUINT64(MF_MT_FRAME_RATE, 257698037761001));
		OnERR_return_NULL(retVal->SetUINT64(MF_MT_PIXEL_ASPECT_RATIO, 4294967297));
		OnERR_return_NULL(retVal->SetUINT32(MF_MT_INTERLACE_MODE, 2));
	}
	else
	{
		OnERR_return_NULL(vInAttrs->CopyAllItems(retVal));
	}

	OnERR_return_NULL(retVal->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
	OnERR_return_NULL(retVal->SetUINT32(MF_MT_VIDEO_PROFILE, 100));
	OnERR_return_NULL(retVal->SetUINT32(MF_MT_VIDEO_LEVEL, 41));
	//OnERR_return_NULL(retVal->SetUINT32(MF_MT_MAX_KEYFRAME_SPACING, 30)); // FIXED IN XMFSinkWriterRep::BeginWriting // DOES NOT WORK WITH IMFCaptureEngine
	OnERR_return_NULL(retVal->SetUINT32(MF_MT_AVG_BITRATE, 6000000)); // 6 megabits

	return retVal;
}