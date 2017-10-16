#include "Topology.h"
#include "TopologyNode.h"
#include "MediaSource.h"
#include "IMFWrapper.h"

#include <mfapi.h>
#include <mfidl.h>

class TopologyRep : public IMFWrapper
{
public:
	TopologyRep();
	~TopologyRep();

	HRESULT								GetLastHRESULT();

	void								CreateAudioPassthroughTopology(MediaSource* audioSource);
	void								CreateVideoPassthroughTopology(MediaSource* videoSource, HWND windowForVideo);
	void								ResolveSingleSourceTopology();
	void								SetTopology(CComPtr<IMFMediaSession> mediaSession);

	CComPtr<IMFTopology>				GetTopology();

private:
	TopologyNode*						CreateAudioRendererNode();
	TopologyNode*						CreateVideoRendererNode(HWND windowForVideo);
	TopologyNode*						CreateNodeFromMediaSource(MediaSource* audioSource);
	void								AddAndConnect2Nodes(CComPtr<IMFTopologyNode> sourceNode, CComPtr<IMFTopologyNode> renderNode);
	bool								IsInputConnected(CComPtr<IMFTopologyNode> node);
	bool								IsOutputConnected(CComPtr<IMFTopologyNode> node);
	void								CleanOutErrors(CComPtr<IMFTopologyNode> node);
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

TopologyNode* TopologyRep::CreateNodeFromMediaSource(MediaSource* audioSource)
{
	TopologyNode* mediaSourceNode = new TopologyNode(audioSource->GetMediaSource());
	if (mediaSourceNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return mediaSourceNode;
}

TopologyNode* TopologyRep::CreateAudioRendererNode()
{
	TopologyNode* audioRendererNode = new TopologyNode(L"SAR");
	if (audioRendererNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return audioRendererNode;
}

TopologyNode* TopologyRep::CreateVideoRendererNode(HWND windowForVideo)
{
	TopologyNode* videoRendererNode = new TopologyNode(windowForVideo);
	if (videoRendererNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return videoRendererNode;
}

void Topology::CreateVideoPassthroughTopology(MediaSource* videoSource, HWND windowForVideo)
{
	m_pRep->CreateVideoPassthroughTopology(videoSource, windowForVideo);
}
void TopologyRep::CreateVideoPassthroughTopology(MediaSource* videoSource, HWND windowForVideo)
{
	TopologyNode* videoSourceNode = CreateNodeFromMediaSource(videoSource);
	if (!videoSourceNode)
	{
		SetLastHR_Fail();
		return;
	}
	TopologyNode* videoRendererNode = CreateVideoRendererNode(windowForVideo);
	if (!videoRendererNode)
	{
		SetLastHR_Fail();
		return;
	}
	AddAndConnect2Nodes(videoSourceNode->GetTopologyNode(), videoRendererNode->GetTopologyNode());
}

void Topology::CreateAudioPassthroughTopology(MediaSource* audioSource)
{
	m_pRep->CreateAudioPassthroughTopology(audioSource);
}
void TopologyRep::CreateAudioPassthroughTopology(MediaSource* audioSource)
{
	TopologyNode* audioSourceNode = CreateNodeFromMediaSource(audioSource);
	if (!audioSourceNode)
	{
		SetLastHR_Fail();
		return;
	}
	TopologyNode* audioRendererNode = CreateAudioRendererNode();
	if (!audioRendererNode)
	{
		SetLastHR_Fail();
		return;
	}
	AddAndConnect2Nodes(audioSourceNode->GetTopologyNode(), audioRendererNode->GetTopologyNode());
}

void TopologyRep::AddAndConnect2Nodes(CComPtr<IMFTopologyNode> sourceNode, CComPtr<IMFTopologyNode> renderNode)
{
	PrintIfErrAndSave(mTopology->AddNode(sourceNode));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(mTopology->AddNode(renderNode));
	}
	if (LastHR_OK())
	{
		PrintIfErrAndSave(sourceNode->ConnectOutput(0, renderNode, 0));
	}
}

void Topology::ResolveSingleSourceTopology()
{
	m_pRep->ResolveSingleSourceTopology();
}
void TopologyRep::ResolveSingleSourceTopology()
{
	CComPtr<IMFTopology> topologyResolved = NULL;
	PrintIfErrAndSave(MFCreateTopology(&topologyResolved));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(topologyResolved->CloneFrom(mTopology));
	}
	WORD nodeCount = 0;
	if (LastHR_OK())
	{
		PrintIfErrAndSave(topologyResolved->GetNodeCount(&nodeCount));
	}
	if (LastHR_OK())
	{
		for (WORD nodeNumber = 0; nodeNumber < nodeCount; nodeNumber++)
		{
			CComPtr<IMFTopologyNode> node = NULL;
			PrintIfErrAndSave(topologyResolved->GetNode(nodeNumber, &node));
			
			CleanOutErrors(node);
			
			bool isConnected = false;
			if (LastHR_OK())
			{
				isConnected = IsInputConnected(node);
			}
			if (LastHR_OK() && isConnected == false)
			{
				isConnected = IsOutputConnected(node);
			}
			if (LastHR_OK() && isConnected == false)
			{
				PrintIfErrAndSave(topologyResolved->RemoveNode(node));
				if (LastHR_OK())
				{
					nodeCount--;
					nodeNumber--;
				}
			}
		}
	}
	if (LastHR_OK())
	{
		mTopology = topologyResolved;
	}
	else
	{
		mTopology = NULL;
	}
}
void TopologyRep::CleanOutErrors(CComPtr<IMFTopologyNode> node)
{
	if (LastHR_OK())
	{
		PrintIfErrAndSave(node->DeleteItem(MF_TOPONODE_ERRORCODE));
	}
	if (LastHR_OK())
	{
		PrintIfErrAndSave(node->DeleteItem(MF_TOPONODE_ERROR_MAJORTYPE));
	}
	if (LastHR_OK())
	{
		PrintIfErrAndSave(node->DeleteItem(MF_TOPONODE_ERROR_SUBTYPE));
	}
}

bool TopologyRep::IsInputConnected(CComPtr<IMFTopologyNode> node)
{
	DWORD inputCount = 0;
	PrintIfErrAndSave(node->GetInputCount(&inputCount));
	if (LastHR_OK())
	{
		for (DWORD inputNumber = 0; inputNumber < inputCount; inputNumber++)
		{
			CComPtr<IMFTopologyNode> pIMFTopologyNodeUp;
			DWORD upIndex;
			PrintIfErrAndSave(node->GetInput(inputNumber, &pIMFTopologyNodeUp, &upIndex));
			if (LastHR_OK())
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
	PrintIfErrAndSave(node->GetOutputCount(&outputCount));
	if (LastHR_OK())
	{
		for (DWORD outputNumber = 0; outputNumber < outputCount; outputNumber++)
		{
			CComPtr<IMFTopologyNode> pIMFTopologyNodeDown;
			DWORD downIndex;
			PrintIfErrAndSave(node->GetOutput(outputNumber, &pIMFTopologyNodeDown, &downIndex));
			if (LastHR_OK())
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
	if (LastHR_OK())
	{
		PrintIfErrAndSave(mTopology->SetUINT32(MF_TOPOLOGY_HARDWARE_MODE, MFTOPOLOGY_HWMODE_USE_HARDWARE));
	}
	if (LastHR_OK())
	{
		PrintIfErrAndSave(mTopology->SetUINT32(MF_TOPOLOGY_DXVA_MODE, MFTOPOLOGY_DXVA_FULL));
	}
	if (LastHR_OK())
	{
		PrintIfErrAndSave(mediaSession->SetTopology(0, mTopology));
	}
}