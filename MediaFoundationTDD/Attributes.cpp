#include "Attributes.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <mfidl.h>

class AttributesRep : public IMFWrapper
{
public:
	AttributesRep();
	~AttributesRep();

	IMFAttributes*	GetAttributes();

	void			CreateVideoDeviceAttributes();

private:
	IMFAttributes*		mAttributesPtr = 0;
};
Attributes::Attributes()
{
	m_pRep = new AttributesRep();
}
AttributesRep::AttributesRep()
{	
}
Attributes::~Attributes()
{
	delete m_pRep;
}
AttributesRep::~AttributesRep()
{

}

IMFAttributes* Attributes::GetAttributes()
{
	return m_pRep->GetAttributes();
}
IMFAttributes* AttributesRep::GetAttributes()
{
	return mAttributesPtr;
}

void Attributes::CreateVideoDeviceAttributes()
{
	return m_pRep->CreateVideoDeviceAttributes();
}
void AttributesRep::CreateVideoDeviceAttributes()
{
	PrintIfErrAndSave(MFCreateAttributes(&mAttributesPtr, 1));
	PrintIfErrAndSave(mAttributesPtr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));
}
