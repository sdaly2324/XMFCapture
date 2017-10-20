#include "TopologyNode.h"
#include "IMFWrapper.h"
#include "SourceReader.h"
#include "PresentationDescriptor.h"
#include "SinkWriter.h"

#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>

class TopologyNodeRep : public IMFWrapper
{
public:
	TopologyNodeRep();
	TopologyNodeRep
	(
		CComPtr<IMFMediaSource> mediaSource, 
		CComPtr<IMFPresentationDescriptor> presentationDescriptor,
		CComPtr<IMFStreamDescriptor> streamDescriptor,
		CComPtr<IMFActivate> renderer
	);
	TopologyNodeRep(std::shared_ptr<SinkWriter> sinkWriter);
	virtual ~TopologyNodeRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopologyNode>			GetTopologyNode();
	CComPtr<IMFTopologyNode>			GetTopologyRendererNode();

private:
	void CreateRendereNode(CComPtr<IMFActivate> device);
	CComPtr<IMFTopologyNode>	mTopologyNode	= NULL;
	CComPtr<IMFTopologyNode>	mRendererNode	= NULL;
};

TopologyNode::TopologyNode()
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep());
}
TopologyNodeRep::TopologyNodeRep()
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &mTopologyNode));
}

TopologyNode::TopologyNode
(
	CComPtr<IMFMediaSource> mediaSource,
	CComPtr<IMFPresentationDescriptor> presentationDescriptor,
	CComPtr<IMFStreamDescriptor> streamDescriptor,
	CComPtr<IMFActivate> renderer
)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(mediaSource, presentationDescriptor, streamDescriptor, renderer));
}
TopologyNodeRep::TopologyNodeRep
(
	CComPtr<IMFMediaSource> mediaSource,
	CComPtr<IMFPresentationDescriptor> presentationDescriptor,
	CComPtr<IMFStreamDescriptor> streamDescriptor,
	CComPtr<IMFActivate> renderer
)
{
	CreateRendereNode(renderer);
	if (LastHR_OK())
	{
		OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &mTopologyNode));
		OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, mediaSource));
		OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presentationDescriptor));
		OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamDescriptor));
	}
}

TopologyNode::TopologyNode(std::shared_ptr<SinkWriter> sinkWriter)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(sinkWriter));
}
TopologyNodeRep::TopologyNodeRep(std::shared_ptr<SinkWriter> sinkWriter)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mTopologyNode));
	OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, sinkWriter->GetMediaSink()));
}

void TopologyNodeRep::CreateRendereNode(CComPtr<IMFActivate> device)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mRendererNode));
	OnERR_return(mRendererNode->SetObject(device));
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

CComPtr<IMFTopologyNode> TopologyNode::GetTopologyRendererNode()
{
	return m_pRep->GetTopologyRendererNode();
}
CComPtr<IMFTopologyNode> TopologyNodeRep::GetTopologyRendererNode()
{
	return mRendererNode;
}