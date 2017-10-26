#include "TopologyNode.h"
#include "MFUtils.h"
#include "PresentationDescriptor.h"
#include "FileSink.h"

#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>

class TopologyNodeRep : public MFUtils
{
public:
	TopologyNodeRep();
	TopologyNodeRep(CComPtr<IMFActivate> renderDevice);
	TopologyNodeRep
	(
		CComPtr<IMFMediaSource> mediaSource, 
		CComPtr<IMFPresentationDescriptor> presentationDescriptor,
		CComPtr<IMFStreamDescriptor> streamDescriptor,
		CComPtr<IMFActivate> renderer
	);
	TopologyNodeRep(std::shared_ptr<FileSink> mediaSink);
	virtual ~TopologyNodeRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopologyNode>			GetNode();
	CComPtr<IMFTopologyNode>			GetRendererNode();

private:
	void CreateRendereNode(CComPtr<IMFActivate> renderDevice);
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

TopologyNode::TopologyNode(CComPtr<IMFActivate> renderDevice)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(renderDevice));
}
TopologyNodeRep::TopologyNodeRep(CComPtr<IMFActivate> renderDevice)
{
	CreateRendereNode(renderDevice);
	mTopologyNode = mRendererNode;
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
	CComPtr<IMFActivate> renderDevice
)
{
	CreateRendereNode(renderDevice);
	if (LastHR_OK())
	{
		OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &mTopologyNode));
		OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, mediaSource));
		OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presentationDescriptor));
		OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamDescriptor));
	}
}

TopologyNode::TopologyNode(std::shared_ptr<FileSink> mediaSink)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(mediaSink));
}
TopologyNodeRep::TopologyNodeRep(std::shared_ptr<FileSink> mediaSink)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mTopologyNode));
	OnERR_return(mTopologyNode->SetObject(mediaSink->GetVideoStreamSink()));
	OnERR_return(mTopologyNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, TRUE));
}

void TopologyNodeRep::CreateRendereNode(CComPtr<IMFActivate> device)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mRendererNode));
	OnERR_return(mRendererNode->SetObject(device));
	OnERR_return(mRendererNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));
	CComPtr<IMFMediaTypeHandler> mediaTypeHandler = NULL;

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
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFTopologyNode> TopologyNode::GetNode()
{
	return m_pRep->GetNode();
}
CComPtr<IMFTopologyNode> TopologyNodeRep::GetNode()
{
	return mTopologyNode;
}

CComPtr<IMFTopologyNode> TopologyNode::GetRendererNode()
{
	return m_pRep->GetRendererNode();
}
CComPtr<IMFTopologyNode> TopologyNodeRep::GetRendererNode()
{
	return mRendererNode;
}