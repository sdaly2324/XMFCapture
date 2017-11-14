#include "TopologyNode.h"
#include "MFUtils.h"
#include "PresentationDescriptor.h"

#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>

class TopologyNodeRep : public MFUtils
{
public:
	TopologyNodeRep(std::wstring name);
	TopologyNodeRep(std::wstring name, CComPtr<IMFTransform> transform);
	TopologyNodeRep(std::wstring name, CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice);
	TopologyNodeRep(std::wstring name, CComPtr<IMFTopologyNode> node);
	TopologyNodeRep(std::wstring name, CComPtr<IMFStreamSink> streamSink);
	TopologyNodeRep
	(
		std::wstring name,
		CComPtr<IMFMediaSource> mediaSource, 
		CComPtr<IMFPresentationDescriptor> presentationDescriptor,
		CComPtr<IMFStreamDescriptor> streamDescriptor,
		CComPtr<IMFActivate> renderer
	);
	virtual ~TopologyNodeRep();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopologyNode>			GetNode();
	CComPtr<IMFTopologyNode>			GetRendererNode();
	CComPtr<IMFMediaType>				GetOutputPrefType();
	CComPtr<IMFMediaType>				GetInputPrefType();

private:
	void DumpFormats();
	CComPtr<IMFTransform> GetTransform();
	CComPtr<IMFMediaTypeHandler> GetMediaTypeHandler(CComPtr<IMFTopologyNode> node);
	CComPtr<IMFMediaType> GetCurrentMediaType(CComPtr<IMFTopologyNode> node);
	void CreateRendereNode(CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice);

	std::wstring				mName = L"";
	CComPtr<IMFTopologyNode>	mTopologyNode	= NULL;
	CComPtr<IMFTopologyNode>	mRendererNode	= NULL;

	bool mDumpFormats = false;
};

void TopologyNodeRep::DumpFormats()
{
	if (!mDumpFormats)
	{
		return;
	}
	CComPtr<IMFMediaType> mediaType = nullptr;
	mTopologyNode->GetInputPrefType(0, &mediaType);
	if (mediaType)
	{
		DumpAttr(mediaType, mName, L"GetInputPrefType");
	}
	mediaType = nullptr;
	mTopologyNode->GetOutputPrefType(0, &mediaType);
	if (mediaType)
	{
		DumpAttr(mediaType, mName, L"GetOutputPrefType");
	}

	CComPtr<IMFMediaTypeHandler> mediaTypeHandler = GetMediaTypeHandler(mTopologyNode);
	if (mediaTypeHandler)
	{
		mediaType = nullptr;
		mediaTypeHandler->GetCurrentMediaType(&mediaType);
		if (mediaType)
		{
			DumpAttr(mediaType, mName, L"GetCurrentMediaType");
		}
	}
	else
	{
		CComPtr<IMFTransform> transform = GetTransform();
		if (transform)
		{
			mediaType = nullptr;
			transform->GetInputCurrentType(0, &mediaType);
			if (mediaType)
			{
				DumpAttr(mediaType, mName, L"GetInputCurrentType");
			}
			mediaType = nullptr;
			transform->GetOutputCurrentType(0, &mediaType);
			if (mediaType)
			{
				DumpAttr(mediaType, mName, L"GetOutputCurrentType");
			}
		}
	}
}

TopologyNode::TopologyNode(std::wstring name)
{
	m_pRep = std::make_unique<TopologyNodeRep>(name);
}
TopologyNodeRep::TopologyNodeRep(std::wstring name) :
	mName(name)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE, &mTopologyNode));
	DumpFormats();
}

TopologyNode::TopologyNode(std::wstring name, CComPtr<IMFStreamSink> streamSink)
{
	m_pRep = std::make_unique<TopologyNodeRep>(name, streamSink);
}
TopologyNodeRep::TopologyNodeRep(std::wstring name, CComPtr<IMFStreamSink> streamSink) :
	mName(name)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mTopologyNode));
	OnERR_return(mTopologyNode->SetObject(streamSink));
	//OnERR_return(mTopologyNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, TRUE));
	DumpFormats();
}

TopologyNode::TopologyNode(std::wstring name, CComPtr<IMFTopologyNode> node)
{
	m_pRep = std::make_unique<TopologyNodeRep>(name, node);
}
TopologyNodeRep::TopologyNodeRep(std::wstring name, CComPtr<IMFTopologyNode> node) :
	mName(name)
{
	mTopologyNode = node;
	DumpFormats();
}
TopologyNode::TopologyNode(std::wstring name, CComPtr<IMFTransform> transform)
{
	m_pRep = std::make_unique<TopologyNodeRep>(name, transform);
}
TopologyNodeRep::TopologyNodeRep(std::wstring name, CComPtr<IMFTransform> transform) :
	mName(name)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &mTopologyNode));
	OnERR_return(mTopologyNode->SetObject(transform));
	DumpFormats();
}

TopologyNode::TopologyNode(std::wstring name, CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice)
{
	m_pRep = std::make_unique<TopologyNodeRep>(name, prefMediaType, renderDevice);
}
TopologyNodeRep::TopologyNodeRep(std::wstring name, CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice) :
	mName(name)
{
	CreateRendereNode(prefMediaType, renderDevice);
	mTopologyNode = mRendererNode;
	DumpFormats();
}

TopologyNode::TopologyNode
(
	std::wstring name,
	CComPtr<IMFMediaSource> mediaSource,
	CComPtr<IMFPresentationDescriptor> presentationDescriptor,
	CComPtr<IMFStreamDescriptor> streamDescriptor,
	CComPtr<IMFActivate> renderer
)
{
	m_pRep = std::make_unique<TopologyNodeRep>(name, mediaSource, presentationDescriptor, streamDescriptor, renderer);
}
TopologyNodeRep::TopologyNodeRep
(
	std::wstring name,
	CComPtr<IMFMediaSource> mediaSource,
	CComPtr<IMFPresentationDescriptor> presentationDescriptor,
	CComPtr<IMFStreamDescriptor> streamDescriptor,
	CComPtr<IMFActivate> renderDevice
) :
	mName(name)
{
	CreateRendereNode(NULL, renderDevice);
	if (LastHR_OK())
	{
		OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &mTopologyNode));
		OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, mediaSource));
		OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presentationDescriptor));
		OnERR_return(mTopologyNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamDescriptor));
		MFTIME mediastart = 0;
		OnERR_return(mTopologyNode->SetUINT64(MF_TOPONODE_MEDIASTART, mediastart));
	}
	DumpFormats();
}

void TopologyNodeRep::CreateRendereNode(CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> device)
{
	OnERR_return(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mRendererNode));
	OnERR_return(mRendererNode->SetObject(device));
	OnERR_return(mRendererNode->SetUINT32(MF_TOPONODE_STREAMID, 0));
	OnERR_return(mRendererNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));


	CComPtr<IMFMediaTypeHandler> mediaTypeHandler = GetMediaTypeHandler(mRendererNode);

	//if (mediaTypeHandler)
	//{
	//	DWORD count = 0;
	//	mediaTypeHandler->GetMediaTypeCount(&count);
	//	for (DWORD index = 0; index < count; index++)
	//	{
	//		CComPtr<IMFMediaType> mediaType = nullptr;
	//		OnERR_return(mediaTypeHandler->GetMediaTypeByIndex(index, &mediaType));
	//		DumpAttr(mediaType, L"CreateRendereNode", std::to_wstring(index));
	//	}
	//}

	if (prefMediaType)
	{
		OnERR_return(mRendererNode->SetInputPrefType(0, prefMediaType));
		//CComPtr<IMFMediaTypeHandler> mediaTypeHandler = GetMediaTypeHandler(mRendererNode);
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
	HRESULT hr = mTopologyNode->GetObject(&unknown);
	if (!SUCCEEDED(hr))
	{
		return nullptr;
	}
	hr = unknown->QueryInterface(IID_IMFTransform, (void**)&retVal);
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