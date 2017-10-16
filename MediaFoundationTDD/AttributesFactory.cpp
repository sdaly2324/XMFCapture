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
	PrintIfErrAndSave(MFCreateAttributes(&retVal, 1));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));
	}
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateAudioDeviceAttributes()
{
	return m_pRep->CreateAudioDeviceAttributes();
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateAudioDeviceAttributes()
{
	CComPtr<IMFAttributes> retVal = NULL;
	PrintIfErrAndSave(MFCreateAttributes(&retVal, 1));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID));
	}
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateSourceReaderAsycCallbackAttributes(IUnknown* callBack)
{
	return m_pRep->CreateSourceReaderAsycCallbackAttributes(callBack);
}
CComPtr<IMFAttributes> AttributesFactoryRep::CreateSourceReaderAsycCallbackAttributes(IUnknown* callBack)
{
	CComPtr<IMFAttributes> retVal = NULL;
	PrintIfErrAndSave(MFCreateAttributes(&retVal, 0));

	if (LastHR_OK() && retVal)
	{
		// for some reason SetUnknown does not count towards the IMFAttributes count
		PrintIfErrAndSave(retVal->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callBack));
	}
	return retVal;
}