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
	TopologyNodeRep(CComPtr<IMFTransform> transform);
	TopologyNodeRep(CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice);
	TopologyNodeRep(CComPtr<IMFTopologyNode> node);
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
	CComPtr<IMFMediaType>				GetOutputPrefType();
	CComPtr<IMFMediaType>				GetInputPrefType();

private:
	CComPtr<IMFTransform> GetTransform();
	CComPtr<IMFMediaTypeHandler> GetMediaTypeHandler(CComPtr<IMFTopologyNode> node);
	CComPtr<IMFMediaType> GetCurrentMediaType(CComPtr<IMFTopologyNode> node);
	void CreateRendereNode(CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice);
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

TopologyNode::TopologyNode(CComPtr<IMFTopologyNode> node)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(node));
}
TopologyNodeRep::TopologyNodeRep(CComPtr<IMFTopologyNode> node)
{
	mTopologyNode = node;
}
TopologyNode::TopologyNode(CComPtr<IMFTransform> transform)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(transform));
}
TopologyNodeRep::TopologyNodeRep(CComPtr<IMFTransform> transform/*, CComPtr<IMFMediaType> inMediaType, CComPtr<IMFMediaType> outMediaType*/)
{
	//CComPtr<IMFAttributes> attributes = nullptr;
	//HRESULT hr = transform->GetInputStreamAttributes(0, &attributes);
	//DumpAttr(attributes, L"", L"");
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &mTopologyNode));
	OnERR_return(mTopologyNode->SetObject(transform));

	//DWORD inputs = 0;
	//hr = mTopologyNode->GetInputCount(&inputs);

	//DWORD outputs = 0;
	//hr = mTopologyNode->GetOutputCount(&outputs);

	//hr = mTopologyNode->SetInputPrefType(0, inMediaType);
	//hr = mTopologyNode->SetOutputPrefType(0, outMediaType);
}

TopologyNode::TopologyNode(CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(prefMediaType, renderDevice));
}
TopologyNodeRep::TopologyNodeRep(CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice)
{
	CreateRendereNode(prefMediaType, renderDevice);
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
	CreateRendereNode(NULL, renderDevice);
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

void TopologyNodeRep::CreateRendereNode(CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> device)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mRendererNode));
	OnERR_return(mRendererNode->SetObject(device));
	OnERR_return(mRendererNode->SetUINT32(MF_TOPONODE_STREAMID, 0));
	OnERR_return(mRendererNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));
	if (prefMediaType)
	{
		OnERR_return(mRendererNode->SetInputPrefType(0, prefMediaType));
		CComPtr<IMFMediaTypeHandler> mediaTypeHandler = GetMediaTypeHandler(mRendererNode);
		if (mediaTypeHandler)
		{
			OnERR_return(mediaTypeHandler->SetCurrentMediaType(prefMediaType));
		}
	}
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


CComPtr<IMFTransform> TopologyNodeRep::GetTransform()
{
	CComPtr<IMFTransform> retVal;
	CComPtr<IUnknown> unknown;
	OnERR_return_NULL(mTopologyNode->GetObject(&unknown));
	HRESULT hr = unknown->QueryInterface(IID_IMFTransform, (void**)&retVal);
	if (!SUCCEEDED(hr))
	{
		CComPtr<IMFActivate> activate;
		OnERR_return_NULL(unknown->QueryInterface(IID_IMFActivate, (void**)&activate));
		OnERR_return_NULL(activate->ActivateObject(IID_IMFTransform, (void**)&retVal));
	}
	return retVal;
}

CComPtr<IMFMediaType> TopologyNode::GetOutputPrefType()
{
	return m_pRep->GetOutputPrefType();
}
CComPtr<IMFMediaType> TopologyNodeRep::GetOutputPrefType()
{
	CComPtr<IMFMediaType> retval = nullptr;
	MF_TOPOLOGY_TYPE nodeType = MF_TOPOLOGY_MAX;
	OnERR_return_NULL(mTopologyNode->GetNodeType(&nodeType));
	if (nodeType == MF_TOPOLOGY_SOURCESTREAM_NODE)
	{
		retval = GetCurrentMediaType(mTopologyNode);
	}
	else if (nodeType == MF_TOPOLOGY_TRANSFORM_NODE)
	{
		CComPtr<IMFTransform> transform = GetTransform();
		if (transform)
		{
			OnERR_return_NULL(transform->GetOutputCurrentType(0, &retval));
		}
	}
	else if (nodeType == MF_TOPOLOGY_TEE_NODE)
	{
		OnERR_return_NULL(mTopologyNode->GetOutputPrefType(0, &retval));
	}
	return retval;
}

CComPtr<IMFMediaType> TopologyNode::GetInputPrefType()
{
	return m_pRep->GetInputPrefType();
}
CComPtr<IMFMediaType> TopologyNodeRep::GetInputPrefType()
{
	CComPtr<IMFMediaType> retval = nullptr;
	MF_TOPOLOGY_TYPE nodeType = MF_TOPOLOGY_MAX;
	OnERR_return_NULL(mTopologyNode->GetNodeType(&nodeType));
	if (nodeType == MF_TOPOLOGY_OUTPUT_NODE)
	{
		retval = GetCurrentMediaType(mTopologyNode);
	}
	else if (nodeType == MF_TOPOLOGY_TRANSFORM_NODE)
	{
		CComPtr<IMFTransform> transform = GetTransform();
		if (transform)
		{
			OnERR_return_NULL(transform->GetInputCurrentType(0, &retval));
		}
	}
	else if (nodeType == MF_TOPOLOGY_TEE_NODE)
	{
		OnERR_return_NULL(mTopologyNode->GetInputPrefType(0, &retval));
	}
	return retval;
}

CComPtr<IMFMediaTypeHandler> TopologyNodeRep::GetMediaTypeHandler(CComPtr<IMFTopologyNode> node)
{
	MF_TOPOLOGY_TYPE nodeType = MF_TOPOLOGY_MAX;
	OnERR_return_NULL(node->GetNodeType(&nodeType));
	CComPtr<IMFMediaTypeHandler> retVal = nullptr;
	if (nodeType == MF_TOPOLOGY_OUTPUT_NODE)
	{
		CComPtr<IUnknown> unknown;
		OnERR_return_NULL(node->GetObject(&unknown));
		CComPtr<IMFStreamSink> streamSink = nullptr;
		HRESULT hr = unknown->QueryInterface(IID_IMFStreamSink, (void**)&streamSink);
		if (!SUCCEEDED(hr))
		{
			CComPtr<IMFActivate> activate = nullptr;
			OnERR_return_NULL(unknown->QueryInterface(IID_IMFActivate, (void**)&activate));
			CComPtr<IMFMediaSink> mediaSink = nullptr;
			OnERR_return_NULL(activate->ActivateObject(IID_IMFMediaSink, (void**)&mediaSink));
			UINT32 streamSinkID = 0;
			OnERR_return_NULL(node->GetUINT32(MF_TOPONODE_STREAMID, &streamSinkID));
			hr = mediaSink->GetStreamSinkById(streamSinkID, &streamSink);
			if (!SUCCEEDED(hr))
			{
				OnERR_return_NULL(mediaSink->AddStreamSink(streamSinkID, NULL, &streamSink));
			}
		}
		OnERR_return_NULL(streamSink->GetMediaTypeHandler(&retVal));
	}
	else if (nodeType == MF_TOPOLOGY_SOURCESTREAM_NODE)
	{
		CComPtr<IMFStreamDescriptor> streamDescriptor = nullptr;
		OnERR_return_NULL(node->GetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, IID_IMFStreamDescriptor, (void**)&streamDescriptor));
		OnERR_return_NULL(streamDescriptor->GetMediaTypeHandler(&retVal));
	}
	return retVal;
}
CComPtr<IMFMediaType> TopologyNodeRep::GetCurrentMediaType(CComPtr<IMFTopologyNode> node)
{
	CComPtr<IMFMediaType> retVal = nullptr;
	CComPtr<IMFMediaTypeHandler> mediaTypeHandler = GetMediaTypeHandler(node);
	if (mediaTypeHandler)
	{
		OnERR_return_NULL(mediaTypeHandler->GetCurrentMediaType(&retVal));
	}
	return retVal;
}