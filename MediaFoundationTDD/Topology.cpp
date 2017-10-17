#include "Topology.h"
#include "TopologyNode.h"
#include "MediaSource.h"
#include "PresentationDescriptor.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <mfidl.h>
#include <memory>

class TopologyRep : public IMFWrapper
{
public:
	TopologyRep();
	~TopologyRep();

	HRESULT								GetLastHRESULT();

	void								CreateAudioPassthroughTopology(std::shared_ptr<MediaSource> audioSource);
	void								CreateVideoPassthroughTopology(std::shared_ptr<MediaSource> videoSource, HWND windowForVideo);
	void								ResolveTopology();
	void								SetTopology(CComPtr<IMFMediaSession> mediaSession);

	CComPtr<IMFTopology>				GetTopology();

private:
	GUID								GetTopSourceNodeMediaType(CComPtr<IMFTopologyNode> topSourceNode);
	void								UpdateSourceNodeMediaTypes(CComPtr<IMFTopologyNode> node);
	bool								IsNodeTypeSource(CComPtr<IMFTopologyNode> node);
	CComPtr<IMFTopology>				ResolveMultiSourceTopology(CComPtr<IMFTopology> topology);
	void								InspectNodeConections(CComPtr<IMFTopology> topology);
	std::shared_ptr<TopologyNode>		CreateAudioRendererNode();
	std::shared_ptr<TopologyNode>		CreateVideoRendererNode(HWND windowForVideo);
	std::shared_ptr<TopologyNode>		CreateNodeFromMediaSource(std::shared_ptr<MediaSource> mediaSource);
	void								AddAndConnect2Nodes(CComPtr<IMFTopologyNode> sourceNode, CComPtr<IMFTopologyNode> renderNode);
	bool								IsInputConnected(CComPtr<IMFTopologyNode> node);
	bool								IsOutputConnected(CComPtr<IMFTopologyNode> node);
	void								CleanOutErrors(CComPtr<IMFTopologyNode> node);
	CComPtr<IMFTopology>				mTopology = NULL;
	WORD								mConnectedNodes = 0;
	bool								mHaveAudioSourceNode = false;
	bool								mHaveVideoSourceNode = false;
};

Topology::Topology()
{
	m_pRep = std::unique_ptr<TopologyRep>(new TopologyRep());
}
TopologyRep::TopologyRep()
{
	OnERR_return(MFCreateTopology(&mTopology));
}
Topology::~Topology()
{
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

std::shared_ptr<TopologyNode> TopologyRep::CreateNodeFromMediaSource(std::shared_ptr<MediaSource> mediaSource)
{
	std::shared_ptr<TopologyNode> mediaSourceNode(new TopologyNode(mediaSource->GetMediaSource()));
	if (mediaSourceNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return mediaSourceNode;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateAudioRendererNode()
{
	std::shared_ptr<TopologyNode> audioRendererNode(new TopologyNode(L"SAR"));
	if (audioRendererNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return audioRendererNode;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateVideoRendererNode(HWND windowForVideo)
{
	std::shared_ptr<TopologyNode> videoRendererNode(new TopologyNode(windowForVideo));
	if (videoRendererNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return videoRendererNode;
}

void Topology::CreateVideoPassthroughTopology(std::shared_ptr<MediaSource> videoSource, HWND windowForVideo)
{
	m_pRep->CreateVideoPassthroughTopology(videoSource, windowForVideo);
}
void TopologyRep::CreateVideoPassthroughTopology(std::shared_ptr<MediaSource> videoSource, HWND windowForVideo)
{
	std::shared_ptr<TopologyNode> videoSourceNode = CreateNodeFromMediaSource(videoSource);
	if (!videoSourceNode)
	{
		SetLastHR_Fail();
		return;
	}
	std::shared_ptr<TopologyNode> videoRendererNode = CreateVideoRendererNode(windowForVideo);
	if (!videoRendererNode)
	{
		SetLastHR_Fail();
		return;
	}
	AddAndConnect2Nodes(videoSourceNode->GetTopologyNode(), videoRendererNode->GetTopologyNode());
}

void Topology::CreateAudioPassthroughTopology(std::shared_ptr<MediaSource> audioSource)
{
	m_pRep->CreateAudioPassthroughTopology(audioSource);
}
void TopologyRep::CreateAudioPassthroughTopology(std::shared_ptr<MediaSource> audioSource)
{
	std::shared_ptr<TopologyNode> audioSourceNode = CreateNodeFromMediaSource(audioSource);
	if (!audioSourceNode)
	{
		SetLastHR_Fail();
		return;
	}
	std::shared_ptr<TopologyNode> audioRendererNode = CreateAudioRendererNode();
	if (!audioRendererNode)
	{
		SetLastHR_Fail();
		return;
	}
	AddAndConnect2Nodes(audioSourceNode->GetTopologyNode(), audioRendererNode->GetTopologyNode());
}

void TopologyRep::AddAndConnect2Nodes(CComPtr<IMFTopologyNode> sourceNode, CComPtr<IMFTopologyNode> renderNode)
{
	OnERR_return(mTopology->AddNode(sourceNode));
	OnERR_return(mTopology->AddNode(renderNode));
	OnERR_return(sourceNode->ConnectOutput(0, renderNode, 0));
}

bool TopologyRep::IsNodeTypeSource(CComPtr<IMFTopologyNode> node)
{
	MF_TOPOLOGY_TYPE nodeType = MF_TOPOLOGY_MAX;
	OnERR_return_false(node->GetNodeType(&nodeType));
	if (nodeType == MF_TOPOLOGY_SOURCESTREAM_NODE)
	{
		return true;
	}
	return false;
}

GUID TopologyRep::GetTopSourceNodeMediaType(CComPtr<IMFTopologyNode> topSourceNode)
{
	CComPtr<IMFMediaSource> mediaSource = NULL;
	if (IsHRError(topSourceNode->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**)&mediaSource)))
	{
		return MFMediaType_Default;
	}
	std::unique_ptr<PresentationDescriptor> presentationDescriptor(new PresentationDescriptor(mediaSource));
	if (presentationDescriptor->GetFirstAudioStreamDescriptor() && LastHR_OK())
	{
		return MFMediaType_Audio;
	}
	if (presentationDescriptor->GetFirstVideoStreamDescriptor() && LastHR_OK())
	{
		return MFMediaType_Video;
	}
	return MFMediaType_Default;
}

void TopologyRep::UpdateSourceNodeMediaTypes(CComPtr<IMFTopologyNode> node)
{
	if (IsNodeTypeSource(node))
	{
		GUID topSourceNodeMediaType = GetTopSourceNodeMediaType(node);
		if (!LastHR_OK())
		{
			return;
		}
		if (topSourceNodeMediaType == MFMediaType_Audio)
		{
			mHaveAudioSourceNode = true;
		}
		else if (topSourceNodeMediaType == MFMediaType_Video)
		{
			mHaveVideoSourceNode = true;
		}
	}
}

void TopologyRep::InspectNodeConections(CComPtr<IMFTopology> topology)
{
	OnERR_return(topology->GetNodeCount(&mConnectedNodes));
	for (WORD nodeNumber = 0; nodeNumber < mConnectedNodes; nodeNumber++)
	{
		CComPtr<IMFTopologyNode> node = NULL;
		OnERR_return(topology->GetNode(nodeNumber, &node));
		CleanOutErrors(node);
		bool isConnected = false;
		if (LastHR_OK())
		{
			isConnected = IsInputConnected(node);
			UpdateSourceNodeMediaTypes(node);
		}
		if (LastHR_OK() && isConnected == false)
		{
			isConnected = IsOutputConnected(node);
		}
		if (LastHR_OK() && isConnected == false)
		{
			OnERR_return(topology->RemoveNode(node));
			mConnectedNodes--;
			nodeNumber--;
		}
	}
}

CComPtr<IMFTopology> TopologyRep::ResolveMultiSourceTopology(CComPtr<IMFTopology> topology)
{
	CComPtr<IMFSequencerSource> sequencerSource = NULL;
	OnERR_return_NULL(MFCreateSequencerSource(NULL, &sequencerSource));
	MFSequencerElementId newMFSequencerElementId = 0;
	OnERR_return_NULL(sequencerSource->AppendTopology(topology, SequencerTopologyFlags_Last, &newMFSequencerElementId));
	CComPtr<IMFMediaSource> mediaSource = NULL;
	OnERR_return_NULL(sequencerSource->QueryInterface(IID_IMFMediaSource, (void**)&mediaSource));
	CComPtr<IMFPresentationDescriptor> presentationDescriptor = NULL;
	OnERR_return_NULL(mediaSource->CreatePresentationDescriptor(&presentationDescriptor));
	CComPtr<IMFMediaSourceTopologyProvider> mediaSourceTopologyProvider = NULL;
	OnERR_return_NULL(sequencerSource->QueryInterface(IID_IMFMediaSourceTopologyProvider, (void**)&mediaSourceTopologyProvider));
	topology.Release();
	OnERR_return_NULL(mediaSourceTopologyProvider->GetMediaSourceTopology(presentationDescriptor, &topology));
	return topology;
}

void Topology::ResolveTopology()
{
	m_pRep->ResolveTopology();
}
void TopologyRep::ResolveTopology()
{
	CComPtr<IMFTopology> topologyClone = NULL;
	OnERR_return(MFCreateTopology(&topologyClone));
	OnERR_return(topologyClone->CloneFrom(mTopology));
	InspectNodeConections(topologyClone);
	if (LastHR_OK() && mHaveAudioSourceNode && mHaveVideoSourceNode)
	{
		topologyClone = ResolveMultiSourceTopology(topologyClone);
	}
	if (LastHR_OK())
	{
		mTopology = topologyClone;
	}
	else
	{
		mTopology = NULL;
	}
}
void TopologyRep::CleanOutErrors(CComPtr<IMFTopologyNode> node)
{
	OnERR_return(node->DeleteItem(MF_TOPONODE_ERRORCODE));
	OnERR_return(node->DeleteItem(MF_TOPONODE_ERROR_MAJORTYPE));
	OnERR_return(node->DeleteItem(MF_TOPONODE_ERROR_SUBTYPE));
}

bool TopologyRep::IsInputConnected(CComPtr<IMFTopologyNode> node)
{
	DWORD inputCount = 0;
	if (!IsHRError(node->GetInputCount(&inputCount)))
	{
		for (DWORD inputNumber = 0; inputNumber < inputCount; inputNumber++)
		{
			CComPtr<IMFTopologyNode> pIMFTopologyNodeUp;
			DWORD upIndex;
			if (!IsHRError(node->GetInput(inputNumber, &pIMFTopologyNodeUp, &upIndex)))
			{
				return true;
			}
		}
	}
	return false;
}

bool TopologyRep::IsOutputConnected(CComPtr<IMFTopologyNode> node)
{
	DWORD outputCount = 0;
	if (!IsHRError(node->GetOutputCount(&outputCount)))
	{
		for (DWORD outputNumber = 0; outputNumber < outputCount; outputNumber++)
		{
			CComPtr<IMFTopologyNode> pIMFTopologyNodeDown;
			DWORD downIndex;
			if (!IsHRError(node->GetOutput(outputNumber, &pIMFTopologyNodeDown, &downIndex)))
			{
				return true;
			}
		}
	}
	return false;
}

void Topology::SetTopology(CComPtr<IMFMediaSession> mediaSession)
{
	m_pRep->SetTopology(mediaSession);
}
void TopologyRep::SetTopology(CComPtr<IMFMediaSession> mediaSession)
{
	OnERR_return(mTopology->SetUINT32(MF_TOPOLOGY_HARDWARE_MODE, MFTOPOLOGY_HWMODE_USE_HARDWARE));
	OnERR_return(mTopology->SetUINT32(MF_TOPOLOGY_DXVA_MODE, MFTOPOLOGY_DXVA_FULL));
	OnERR_return(mediaSession->SetTopology(0, mTopology));
}