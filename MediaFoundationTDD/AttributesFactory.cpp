#include "AttributesFactory.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class AttributesFactoryRep : public IMFWrapper
{
public:
	AttributesFactoryRep();
	~AttributesFactoryRep();

	HRESULT GetLastHRESULT();

	CComPtr<IMFAttributes>	CreateVideoDeviceAttributes();
	CComPtr<IMFAttributes>	CreateAudioDeviceAttributes();
	CComPtr<IMFAttributes>	CreateSourceReaderAsycCallbackAttributes(IUnknown* callBack);
	CComPtr<IMFAttributes>	CreateSinkWriterAttributes();

private:
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
	return IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFAttributes> AttributesFactory::CreateVideoDeviceAttributes()
{
	return m_pRep->CreateVideoDeviceAttributes();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateVideoDeviceAttributes()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 1));
	OnERR_return_NULL(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateAudioDeviceAttributes()
{
	return m_pRep->CreateAudioDeviceAttributes();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateAudioDeviceAttributes()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 1));
	OnERR_return_NULL(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateSourceReaderAsycCallbackAttributes(IUnknown* callBack)
{
	return m_pRep->CreateSourceReaderAsycCallbackAttributes(callBack);
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateSourceReaderAsycCallbackAttributes(IUnknown* callBack)
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 0));
		// for some reason SetUnknown does not count towards the IMFAttributes count
	OnERR_return_NULL(retVal->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callBack));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateSinkWriterAttributes()
{
	return m_pRep->CreateSinkWriterAttributes();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateSinkWriterAttributes()
{
	CComPtr<IMFAttributes> attributes = NULL;
	OnERR_return_NULL(MFCreateAttributes(&attributes, 0));
	OnERR_return_NULL(attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	OnERR_return_NULL(attributes->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Stream));
	OnERR_return_NULL(attributes->SetGUID(MF_MT_SUBTYPE, MFStreamFormat_MPEG2Transport));
	return attributes;
}