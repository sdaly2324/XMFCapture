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

	IMFAttributes*	CreateVideoDeviceAttributes();
	IMFAttributes*	CreateAudioDeviceAttributes();
	IMFAttributes*	CreateSourceReaderAsycCallbackAttributes(IUnknown* callBack);

private:
};
AttributesFactory::AttributesFactory()
{
	m_pRep = new AttributesFactoryRep();
}
AttributesFactoryRep::AttributesFactoryRep()
{	
}
AttributesFactory::~AttributesFactory()
{
	delete m_pRep;
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

IMFAttributes* AttributesFactory::CreateVideoDeviceAttributes()
{
	return m_pRep->CreateVideoDeviceAttributes();
}
IMFAttributes* AttributesFactoryRep::CreateVideoDeviceAttributes()
{
	IMFAttributes* retVal = NULL;
	PrintIfErrAndSave(MFCreateAttributes(&retVal, 1));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));
	}
	return retVal;
}

IMFAttributes* AttributesFactory::CreateAudioDeviceAttributes()
{
	return m_pRep->CreateAudioDeviceAttributes();
}
IMFAttributes* AttributesFactoryRep::CreateAudioDeviceAttributes()
{
	IMFAttributes* retVal = NULL;
	PrintIfErrAndSave(MFCreateAttributes(&retVal, 1));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID));
	}
	return retVal;
}

IMFAttributes* AttributesFactory::CreateSourceReaderAsycCallbackAttributes(IUnknown* callBack)
{
	return m_pRep->CreateSourceReaderAsycCallbackAttributes(callBack);
}
IMFAttributes* AttributesFactoryRep::CreateSourceReaderAsycCallbackAttributes(IUnknown* callBack)
{
	IMFAttributes* retVal = NULL;
	PrintIfErrAndSave(MFCreateAttributes(&retVal, 0));

	if (LastHR_OK() && retVal)
	{
		// for some reason SetUnknown does not count towards the IMFAttributes count
		PrintIfErrAndSave(retVal->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, callBack));
	}
	return retVal;
}