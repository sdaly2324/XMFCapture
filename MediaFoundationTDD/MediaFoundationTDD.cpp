#include "MediaFoundationTDD.h"
#include "IMFWrapper.h"
#include "AttributesFactory.h"
#include "Devices.h"

#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <vector>

class MediaFoundationTDDRep : public IMFWrapper
{
public:
	MediaFoundationTDDRep();
	~MediaFoundationTDDRep();

	HRESULT						GetLastHRESULT();

	void						CreateTopology();
	CComPtr<IMFTopology>		GetTopology();

private:

	CComPtr<IMFTopology>		mTopologyPtr		= NULL;
};

MediaFoundationTDD::MediaFoundationTDD()
{
	m_pRep = new MediaFoundationTDDRep();
}
MediaFoundationTDDRep::MediaFoundationTDDRep()
{
	MFStartup(MF_VERSION);
}
MediaFoundationTDD::~MediaFoundationTDD()
{
	delete m_pRep;
}
MediaFoundationTDDRep::~MediaFoundationTDDRep()
{
	delete mTopologyPtr;
}

HRESULT MediaFoundationTDD::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT MediaFoundationTDDRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}

void MediaFoundationTDD::CreateTopology()
{
	return m_pRep->CreateTopology();
}
void MediaFoundationTDDRep::CreateTopology()
{
	PrintIfErrAndSave(MFCreateTopology(&mTopologyPtr));
}
CComPtr<IMFTopology> MediaFoundationTDD::GetTopology()
{
	return m_pRep->GetTopology();
}
CComPtr<IMFTopology> MediaFoundationTDDRep::GetTopology()
{
	return mTopologyPtr;
}