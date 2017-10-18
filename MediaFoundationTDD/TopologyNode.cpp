#include "TopologyNode.h"
#include "IMFWrapper.h"
#include "SourceReader.h"
#include "PresentationDescriptor.h"
#include "Devices.h"

#include <mfidl.h>

class TopologyNodeRep : public IMFWrapper
{
public:
	TopologyNodeRep(CComPtr<IMFMediaSource> mediaSource);
	TopologyNodeRep(CComPtr<IMFActivate> device);
	~TopologyNodeRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopologyNode>			GetTopologyNode();

private:
	void CreateAudioRenderer();
	CComPtr<IMFTopologyNode> mTopologyNode = NULL;
};

TopologyNode::TopologyNode(CComPtr<IMFMediaSource> mediaSource)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(mediaSource));
}
TopologyNodeRep::TopologyNodeRep(CComPtr<IMFMediaSource> mediaSource)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &mTopologyNode));
	OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, mediaSource));
	std::unique_ptr<PresentationDescriptor> presentationDescriptor(new PresentationDescriptor(mediaSource));
	if (presentationDescriptor->GetLastHRESULT() != S_OK || !presentationDescriptor)
	{
		SetLastHR_Fail();
		return;
	}
	OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presentationDescriptor->GetPresentationDescriptor()));

	CComPtr<IMFStreamDescriptor> firstVideoStreamDescriptor = presentationDescriptor->GetFirstVideoStreamDescriptor();
	CComPtr<IMFStreamDescriptor> firstAudioStreamDescriptor = presentationDescriptor->GetFirstAudioStreamDescriptor();
	if (firstVideoStreamDescriptor && firstAudioStreamDescriptor)
	{
		OutputDebugStringW(L"TopologyNodeRep::TopologyNodeRep failed because we found 2 IMFStreamDescriptors instead of the expected 1");
		mTopologyNode = NULL;
		return;
	}
	CComPtr<IMFStreamDescriptor> streamDescriptorToUse = NULL;
	if (firstVideoStreamDescriptor)
	{
		streamDescriptorToUse = firstVideoStreamDescriptor;
	}
	else if(firstAudioStreamDescriptor)
	{
		streamDescriptorToUse = firstAudioStreamDescriptor;
	}
	else
	{
		OutputDebugStringW(L"TopologyNodeRep::TopologyNodeRep failed to find a IMFStreamDescriptor");
		mTopologyNode = NULL;
		return;
	}
	OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamDescriptorToUse));
}

TopologyNode::TopologyNode(CComPtr<IMFActivate> device)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(device));
}
TopologyNodeRep::TopologyNodeRep(CComPtr<IMFActivate> device)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mTopologyNode));
	OnERR_return(mTopologyNode->SetObject(device));
}

TopologyNode::~TopologyNode()
{
}
TopologyNodeRep::~TopologyNodeRep()
{
}

HRESULT TopologyNode::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT TopologyNodeRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFTopologyNode> TopologyNode::GetTopologyNode()
{
	return m_pRep->GetTopologyNode();
}
CComPtr<IMFTopologyNode> TopologyNodeRep::GetTopologyNode()
{
	return mTopologyNode;
}