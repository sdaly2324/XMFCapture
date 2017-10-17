#include "TopologyNode.h"
#include "IMFWrapper.h"
#include "SourceReader.h"
#include "PresentationDescriptor.h"

#include <mfidl.h>

class TopologyNodeRep : public IMFWrapper
{
public:
	TopologyNodeRep(CComPtr<IMFMediaSource> mediaSource);
	TopologyNodeRep(HWND windowForVideo);
	TopologyNodeRep(std::wstring nodeType);
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
	PrintIfErrAndSave(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &mTopologyNode));
	if (!LastHR_OK())
	{
		return;
	}
	PrintIfErrAndSave(mTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, mediaSource));
	if (!LastHR_OK())
	{
		return;
	}
	std::unique_ptr<PresentationDescriptor> presentationDescriptor(new PresentationDescriptor(mediaSource));
	if (presentationDescriptor->GetLastHRESULT() != S_OK || !presentationDescriptor)
	{
		SetLastHR_Fail();
		return;
	}
	PrintIfErrAndSave(mTopologyNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, presentationDescriptor->GetPresentationDescriptor()));

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
	PrintIfErrAndSave(mTopologyNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, streamDescriptorToUse));
}

TopologyNode::TopologyNode(HWND windowForVideo)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(windowForVideo));
}
TopologyNodeRep::TopologyNodeRep(HWND windowForVideo)
{
	CComPtr<IMFActivate> pIMFActivate = NULL;
	PrintIfErrAndSave(MFCreateVideoRendererActivate(windowForVideo, &pIMFActivate));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mTopologyNode));
	}
	if (LastHR_OK())
	{
		PrintIfErrAndSave(mTopologyNode->SetObject(pIMFActivate));
	}
}

TopologyNode::TopologyNode(std::wstring nodeType)
{
	m_pRep = std::unique_ptr<TopologyNodeRep>(new TopologyNodeRep(nodeType));
}
TopologyNodeRep::TopologyNodeRep(std::wstring nodeType)
{
	if (nodeType == L"SAR")
	{
		CreateAudioRenderer();
	}
	else
	{
		SetLastHR_Fail();
	}
}
void TopologyNodeRep::CreateAudioRenderer()
{
	CComPtr<IMFActivate> pIMFActivate = NULL;
	PrintIfErrAndSave(MFCreateAudioRendererActivate(&pIMFActivate));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mTopologyNode));
	}
	if (LastHR_OK())
	{
		PrintIfErrAndSave(mTopologyNode->SetObject(pIMFActivate));
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