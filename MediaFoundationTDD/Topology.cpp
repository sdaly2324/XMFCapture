#include "Topology.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <mfidl.h>

class TopologyRep : public IMFWrapper
{
public:
	TopologyRep();
	~TopologyRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopology>				GetTopology();

private:
	CComPtr<IMFTopology>				mTopology = NULL;
};

Topology::Topology()
{
	m_pRep = new TopologyRep();
}
TopologyRep::TopologyRep()
{
	PrintIfErrAndSave(MFCreateTopology(&mTopology));
}
Topology::~Topology()
{
	delete m_pRep;
}
TopologyRep::~TopologyRep()
{
}

HRESULT Topology::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT TopologyRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}


CComPtr<IMFTopology> Topology::GetTopology()
{
	return m_pRep->GetTopology();
}
CComPtr<IMFTopology> TopologyRep::GetTopology()
{
	return mTopology;
}