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

	CComPtr<IMFAttributes>	CreateVDeviceAttrs();
	CComPtr<IMFAttributes>	CreateADeviceAttrs();
	CComPtr<IMFAttributes>	CreateSReaderCbAttrs(IUnknown* callBack);
	CComPtr<IMFAttributes>	CreateFSinkAttrs();
	CComPtr<IMFAttributes>	CreateVOutAttrs(CComPtr<IMFAttributes> vInAttrs);
	CComPtr<IMFAttributes>	CreateAOutAttrs();

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

CComPtr<IMFAttributes> AttributesFactory::CreateVDeviceAttrs()
{
	return m_pRep->CreateVDeviceAttrs();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateVDeviceAttrs()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 1));
	OnERR_return_NULL(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateADeviceAttrs()
{
	return m_pRep->CreateADeviceAttrs();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateADeviceAttrs()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 1));
	OnERR_return_NULL(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateSReaderCbAttrs(IUnknown* callBack)
{
	return m_pRep->CreateSReaderCbAttrs(callBack);
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateSReaderCbAttrs(IUnknown* callBack)
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 0));
		// for some reason SetUnknown does not count towards the IMFAttributes count
	OnERR_return_NULL(retVal->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callBack));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateFSinkAttrs()
{
	return m_pRep->CreateFSinkAttrs();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateFSinkAttrs()
{
	CComPtr<IMFAttributes> attributes = NULL;
	OnERR_return_NULL(MFCreateAttributes(&attributes, 0));
	//OnERR_return_NULL(attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	OnERR_return_NULL(attributes->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Stream));
	OnERR_return_NULL(attributes->SetGUID(MF_MT_SUBTYPE, MFStreamFormat_MPEG2Transport));
	return attributes;
}

CComPtr<IMFAttributes> AttributesFactory::CreateAOutAttrs()
{
	return m_pRep->CreateAOutAttrs();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateAOutAttrs()
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
CComPtr<IMFAttributes> AttributesFactory::CreateVOutAttrs(CComPtr<IMFAttributes> vInAttrs)
{
	return m_pRep->CreateVOutAttrs(vInAttrs);
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateVOutAttrs(CComPtr<IMFAttributes> vInAttrs)
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 0));
	OnERR_return_NULL(retVal->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
	OnERR_return_NULL(CopyAttribute(vInAttrs, retVal, MF_MT_FRAME_SIZE));
	OnERR_return_NULL(CopyAttribute(vInAttrs, retVal, MF_MT_FRAME_RATE));
	OnERR_return_NULL(CopyAttribute(vInAttrs, retVal, MF_MT_PIXEL_ASPECT_RATIO));
	OnERR_return_NULL(CopyAttribute(vInAttrs, retVal, MF_MT_INTERLACE_MODE));
	OnERR_return_NULL(retVal->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
	OnERR_return_NULL(retVal->SetUINT32(MF_MT_VIDEO_PROFILE, 100));
	OnERR_return_NULL(retVal->SetUINT32(MF_MT_VIDEO_LEVEL, 41));
	//OnERR_return_NULL(retVal->SetUINT32(MF_MT_MAX_KEYFRAME_SPACING, 30); // FIXED IN XMFSinkWriterRep::BeginWriting // DOES NOT WORK WITH IMFCaptureEngine
	OnERR_return_NULL(retVal->SetUINT32(MF_MT_AVG_BITRATE, 6000000)); // 6 megabits

	return retVal;
}