#include "Topology.h"
#include "TopologyNode.h"
#include "MediaSource.h"
#include "PresentationDescriptor.h"
#include "MFUtils.h"
#include "TranscodeProfileFactory.h"

#include <mfapi.h>
#include <mfidl.h>
#include <strmif.h>
#include <codecapi.h>
#include <memory>
#include <vector>

class TopologyRep : public MFUtils
{
public:
	TopologyRep();
	~TopologyRep();

	HRESULT GetLastHRESULT();

	void CreatePassthroughTopology
	(
		CComPtr<IMFMediaSource> mediaSource,
		CComPtr<IMFActivate> videoRendererDevice,
		CComPtr<IMFActivate> audioRendererDevice
	);
	void CreateCaptureAndPassthroughTopology
	(
		CComPtr<IMFMediaSource> mediaSource,
		CComPtr<IMFActivate> videoRendererDevice,
		CComPtr<IMFActivate> audioRendererDevice,
		std::shared_ptr<FileSink> mediaSink
	);
	void CreateVideoOnlyCaptureTopology(std::shared_ptr<MediaSource> mediaSource, const std::wstring& fileToWrite);
	void CreateAudioOnlyCaptureTopology(std::shared_ptr<MediaSource> mediaSource, const std::wstring& fileToWrite);
	void CreateAudioOnlyCaptureAndPassthroughTopology
	(
		std::shared_ptr<MediaSource> mediaSource,
		const std::wstring& fileToWrite,
		CComPtr<IMFActivate> audioRendererDevice
	);
	void ResolveTopology();
	void SetTopology(CComPtr<IMFMediaSession> mediaSession);

	CComPtr<IMFTopology> GetTopology();

private:
	void						InsertTEEAudioRenderer(CComPtr<IMFActivate> audioRendererDevice);
	CComPtr<IMFTopologyNode>	GetFirstNodeTypeFromTopology(MF_TOPOLOGY_TYPE nodeType);
	CComPtr<IMFTopologyNode>	GetSourceNodeFromTopology();
	CComPtr<IMFTopologyNode>	GetTransformNodeFromTopology();
	CComPtr<IMFTopologyNode>	GetOutputNodeFromTopology();

	GUID											GetTopSourceNodeMediaType(CComPtr<IMFTopologyNode> topSourceNode);
	void											UpdateSourceNodeMediaTypes(CComPtr<IMFTopologyNode> node);
	bool											IsNodeTypeSource(CComPtr<IMFTopologyNode> node);
	CComPtr<IMFTopology>							ResolveMultiSourceTopology(CComPtr<IMFTopology> topology);
	void											BindOutputNode(CComPtr<IMFTopologyNode> node);
	void											BindOutputNodes(CComPtr<IMFTopology> topology);
	void											InspectNodeConections(CComPtr<IMFTopology> topology);
	std::shared_ptr<TopologyNode>					CreateTeeNode();
	std::shared_ptr<TopologyNode>					CreateSinkNode(std::shared_ptr<FileSink> mediaSink);
	std::shared_ptr<TopologyNode>					CreateRendererNode(CComPtr<IMFActivate> renderDevice);
	std::vector< std::shared_ptr< TopologyNode> >	CreateSourceNodes
													(
														CComPtr<IMFMediaSource> mediaSource, 
														CComPtr<IMFActivate> videoRendererDevice,
														CComPtr<IMFActivate> audioRendererDevice
													);
	std::shared_ptr<TopologyNode>					CreateAudioSourceNode
													(
														CComPtr<IMFMediaSource> mediaSource, 
														std::shared_ptr<PresentationDescriptor> presentationDescriptor,
														CComPtr<IMFActivate> renderer
													);
	std::shared_ptr<TopologyNode>					CreateVideoSourceNode
													(
														CComPtr<IMFMediaSource> mediaSource, 
														std::shared_ptr<PresentationDescriptor> presentationDescriptor,
														CComPtr<IMFActivate> renderer
													);
	std::shared_ptr<TopologyNode>					CreateSourceNode
													(
														CComPtr<IMFMediaSource> mediaSource, 
														CComPtr<IMFPresentationDescriptor> presentationDescriptor, 
														CComPtr<IMFStreamDescriptor> streamDescriptor,
														CComPtr<IMFActivate> renderer
													);
	bool											NodeExists(CComPtr<IMFTopologyNode> node);
	HRESULT											AddAndConnect2Nodes(CComPtr<IMFTopologyNode> inputNode, CComPtr<IMFTopologyNode> outputNode, DWORD outputIndex);
	bool											IsInputConnected(CComPtr<IMFTopologyNode> node);
	bool											IsOutputConnected(CComPtr<IMFTopologyNode> node);
	void											CleanOutErrors(CComPtr<IMFTopologyNode> node);

	CComPtr<IMFTopology>				mTopology = NULL;
	bool								mHaveAudioSourceNode = false;
	bool								mHaveVideoSourceNode = false;
};

Topology::Topology()
{
	m_pRep = std::unique_ptr<TopologyRep>(new TopologyRep());
}
TopologyRep::TopologyRep()
{
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
	return MFUtils::GetLastHRESULT();
}


CComPtr<IMFTopology> Topology::GetTopology()
{
	return m_pRep->GetTopology();
}
CComPtr<IMFTopology> TopologyRep::GetTopology()
{
	return mTopology;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateSourceNode
(
	CComPtr<IMFMediaSource> mediaSource, 
	CComPtr<IMFPresentationDescriptor> presentationDescriptor, 
	CComPtr<IMFStreamDescriptor> streamDescriptor,
	CComPtr<IMFActivate> renderer
)
{
	std::shared_ptr<TopologyNode> mediaSourceNode(new TopologyNode(mediaSource, presentationDescriptor, streamDescriptor, renderer));
	if (mediaSourceNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return mediaSourceNode;
}
std::shared_ptr<TopologyNode> TopologyRep::CreateAudioSourceNode
(
	CComPtr<IMFMediaSource> mediaSource, 
	std::shared_ptr<PresentationDescriptor> presentationDescriptor,
	CComPtr<IMFActivate> renderer
)
{
	CComPtr<IMFStreamDescriptor> audioStreamDescriptor = presentationDescriptor->GetFirstAudioStreamDescriptor();
	if (audioStreamDescriptor)
	{
		return CreateSourceNode(mediaSource, presentationDescriptor->GetPresentationDescriptor(), audioStreamDescriptor, renderer);
	}
	return NULL;
}
std::shared_ptr<TopologyNode> TopologyRep::CreateVideoSourceNode
(
	CComPtr<IMFMediaSource> mediaSource, 
	std::shared_ptr<PresentationDescriptor> presentationDescriptor,
	CComPtr<IMFActivate> renderer
)
{
	CComPtr<IMFStreamDescriptor> videoStreamDescriptor = presentationDescriptor->GetFirstVideoStreamDescriptor();
	if (videoStreamDescriptor)
	{
		return CreateSourceNode(mediaSource, presentationDescriptor->GetPresentationDescriptor(), videoStreamDescriptor, renderer);
	}
	return NULL;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateTeeNode()
{
	std::shared_ptr<TopologyNode> teeNode(new TopologyNode());
	if (teeNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return teeNode;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateRendererNode(CComPtr<IMFActivate> renderDevice)
{
	std::shared_ptr<TopologyNode> rendererNode(new TopologyNode(renderDevice));
	if (rendererNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return rendererNode;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateSinkNode(std::shared_ptr<FileSink> mediaSink)
{
	std::shared_ptr<TopologyNode> sinkNode(new TopologyNode(mediaSink));
	if (sinkNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return sinkNode;
}

std::vector< std::shared_ptr< TopologyNode> > TopologyRep::CreateSourceNodes
(
	CComPtr<IMFMediaSource> mediaSource, 
	CComPtr<IMFActivate> videoRendererDevice,
	CComPtr<IMFActivate> audioRendererDevice
)
{
	std::vector< std::shared_ptr< TopologyNode> > retVal;

	std::shared_ptr<PresentationDescriptor> mediaSourcePresentationDescriptor(new PresentationDescriptor(mediaSource));
	//DumpAttr(mediaSourcePresentationDescriptor->GetPresentationDescriptor(), L"", L"");
	std::shared_ptr<TopologyNode> audioSourceNode = CreateAudioSourceNode(mediaSource, mediaSourcePresentationDescriptor, audioRendererDevice);
	if (audioSourceNode)
	{
		retVal.push_back(audioSourceNode);
	}

	std::shared_ptr<TopologyNode> videoSourceNode = CreateVideoSourceNode(mediaSource, mediaSourcePresentationDescriptor, videoRendererDevice);
	if (videoSourceNode)
	{
		retVal.push_back(videoSourceNode);
	}

	return retVal;
}

void Topology::CreateCaptureAndPassthroughTopology
(
	CComPtr<IMFMediaSource> mediaSource,
	CComPtr<IMFActivate> videoRendererDevice,
	CComPtr<IMFActivate> audioRendererDevice,
	std::shared_ptr<FileSink> mediaSink
)
{
	m_pRep->CreateCaptureAndPassthroughTopology(mediaSource, videoRendererDevice, audioRendererDevice, mediaSink);
}
void TopologyRep::CreateCaptureAndPassthroughTopology
(
	CComPtr<IMFMediaSource> mediaSource,
	CComPtr<IMFActivate> videoRendererDevice,
	CComPtr<IMFActivate> audioRendererDevice,
	std::shared_ptr<FileSink> mediaSink
)
{
	if (!mTopology)
	{
		OnERR_return(MFCreateTopology(&mTopology));
	}
	CComPtr<IMFTopologyNode> sinkNode = CreateSinkNode(mediaSink)->GetNode();
	if (!sinkNode)
	{
		SetLastHR_Fail();
		return;
	}
	std::vector< std::shared_ptr< TopologyNode> > sourceNodes = CreateSourceNodes(mediaSource, videoRendererDevice, audioRendererDevice);
	if (sourceNodes.empty())
	{
		SetLastHR_Fail();
		return;
	}
	for (auto& sourceNode : sourceNodes)
	{
		//CComPtr<IMFTopologyNode> teeNode = CreateTeeNode()->GetNode();
		//if (!teeNode)
		//{
		//	SetLastHR_Fail();
		//	return;
		//}
		CComPtr<IMFTopologyNode> rendererNode = sourceNode->GetRendererNode();
		if (!rendererNode)
		{
			SetLastHR_Fail();
			return;
		}
		//OnERR_return(AddAndConnect2Nodes(sourceNode->GetNode(), teeNode, 0));
		//OnERR_return(AddAndConnect2Nodes(teeNode, sinkNode, 0));
		//OnERR_return(AddAndConnect2Nodes(teeNode, rendererNode, 1));

		OnERR_return(AddAndConnect2Nodes(sourceNode->GetNode(), sinkNode, 0));
	}
}

void Topology::CreatePassthroughTopology
(
	CComPtr<IMFMediaSource> mediaSource, 
	CComPtr<IMFActivate> videoRendererDevice,
	CComPtr<IMFActivate> audioRendererDevice
)
{
	m_pRep->CreatePassthroughTopology(mediaSource, videoRendererDevice, audioRendererDevice);
}
void TopologyRep::CreatePassthroughTopology
(
	CComPtr<IMFMediaSource> mediaSource, 
	CComPtr<IMFActivate> videoRendererDevice,
	CComPtr<IMFActivate> audioRendererDevice
)
{
	if (!mTopology)
	{
		OnERR_return(MFCreateTopology(&mTopology));
	}
	std::vector< std::shared_ptr< TopologyNode> > sourceNodes = CreateSourceNodes(mediaSource, videoRendererDevice, audioRendererDevice);
	for (auto& sourceNode : sourceNodes)
	{
		CComPtr<IMFTopologyNode> rendererNode = sourceNode->GetRendererNode();
		if (!rendererNode)
		{
			SetLastHR_Fail();
			return;
		}
		OnERR_return(AddAndConnect2Nodes(sourceNode->GetNode(), rendererNode, 0));
	}
}

bool TopologyRep::NodeExists(CComPtr<IMFTopologyNode> node)
{
	TOPOID nodeID = 0;
	OnERR_return_HR(node->GetTopoNodeID(&nodeID));
	CComPtr<IMFTopologyNode> nodeNodeExist = NULL;
	HRESULT hr = mTopology->GetNodeByID(nodeID, &nodeNodeExist);
	if (nodeNodeExist)
	{
		return true;
	}
	return false;
}
HRESULT TopologyRep::AddAndConnect2Nodes(CComPtr<IMFTopologyNode> inputNode, CComPtr<IMFTopologyNode> outputNode, DWORD outputIndex)
{
	if (!NodeExists(inputNode))
	{
		OnERR_return_HR(mTopology->AddNode(inputNode));
	}
	if (!NodeExists(outputNode))
	{
		OnERR_return_HR(mTopology->AddNode(outputNode));
	}
	OnERR_return_HR(inputNode->ConnectOutput(outputIndex, outputNode, 0));
	return S_OK;
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

void TopologyRep::BindOutputNode(CComPtr<IMFTopologyNode> node)
{
	CComPtr<IUnknown> nodeObject = NULL;
	OnERR_return(node->GetObject(&nodeObject));
	CComPtr<IMFActivate> activate = NULL;
	HRESULT hr = nodeObject->QueryInterface(IID_PPV_ARGS(&activate));
	CComPtr<IMFStreamSink> streamSink = NULL;
	if (SUCCEEDED(hr))
	{
		CComPtr<IMFMediaSink> mediaSink = NULL;
		OnERR_return(activate->ActivateObject(IID_PPV_ARGS(&mediaSink)));
		DWORD streamID = MFGetAttributeUINT32(node, MF_TOPONODE_STREAMID, 0);
		hr = mediaSink->GetStreamSinkById(streamID, &streamSink);
		if (FAILED(hr))
		{
			OnERR_return(mediaSink->AddStreamSink(streamID, NULL, &streamSink));
			hr = S_OK;
		}
		if (SUCCEEDED(hr))
		{
			OnERR_return(node->SetObject(streamSink));
		}
	}
	else
	{
		OnERR_return(nodeObject->QueryInterface(IID_PPV_ARGS(&streamSink)));
	}
}

void TopologyRep::BindOutputNodes(CComPtr<IMFTopology> topology)
{
	CComPtr<IMFCollection> collection = NULL;
	OnERR_return(topology->GetOutputNodeCollection(&collection));
	DWORD outputNodes = 0;
	OnERR_return(collection->GetElementCount(&outputNodes));
	for (WORD nodeNumber = 0; nodeNumber < outputNodes; nodeNumber++)
	{
		CComPtr<IUnknown> unknown = NULL;
		OnERR_return(collection->GetElement(nodeNumber, &unknown));
		CComPtr<IMFTopologyNode> node = NULL;
		OnERR_return(unknown->QueryInterface(IID_IMFTopologyNode, (void**)&node));
		BindOutputNode(node);
	}
}

void TopologyRep::InspectNodeConections(CComPtr<IMFTopology> topology)
{
	WORD connectedNodes = 0;
	OnERR_return(topology->GetNodeCount(&connectedNodes));
	if (connectedNodes == 0)
	{
		SetLastHR_Fail();
		return;
	}
	for (WORD nodeNumber = 0; nodeNumber < connectedNodes; nodeNumber++)
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
			connectedNodes--;
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
	//BindOutputNodes(topologyClone);
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

void Topology::CreateVideoOnlyCaptureTopology(std::shared_ptr<MediaSource> mediaSource, const std::wstring& fileToWrite)
{
	m_pRep->CreateVideoOnlyCaptureTopology(mediaSource, fileToWrite);
}
void TopologyRep::CreateVideoOnlyCaptureTopology(std::shared_ptr<MediaSource> mediaSource, const std::wstring& fileToWrite)
{
	TranscodeProfileFactory transcodeProfileFactory;
	CComPtr<IMFAttributes> attrs = NULL; 
	//attrs = mediaSource->GetVideoMediaType();
	CComPtr<IMFTranscodeProfile> transcodeProfile = transcodeProfileFactory.CreateVideoOnlyTranscodeProfile(attrs);
	OnERR_return(MFCreateTranscodeTopology(mediaSource->GetMediaSource(), fileToWrite.c_str(), transcodeProfile, &mTopology));
}

void Topology::CreateAudioOnlyCaptureTopology(std::shared_ptr<MediaSource> mediaSource, const std::wstring& fileToWrite)
{
	m_pRep->CreateAudioOnlyCaptureTopology(mediaSource, fileToWrite);
}
void TopologyRep::CreateAudioOnlyCaptureTopology(std::shared_ptr<MediaSource> mediaSource, const std::wstring& fileToWrite)
{
	TranscodeProfileFactory transcodeProfileFactory;
	CComPtr<IMFTranscodeProfile> transcodeProfile = transcodeProfileFactory.CreateAudioOnlyTranscodeProfile();
	OnERR_return(MFCreateTranscodeTopology(mediaSource->GetMediaSource(), fileToWrite.c_str(), transcodeProfile, &mTopology));
}

void Topology::CreateAudioOnlyCaptureAndPassthroughTopology
(
	std::shared_ptr<MediaSource> mediaSource,
	const std::wstring& fileToWrite,
	CComPtr<IMFActivate> audioRendererDevice
)
{
	m_pRep->CreateAudioOnlyCaptureAndPassthroughTopology(mediaSource, fileToWrite, audioRendererDevice);
}
void TopologyRep::CreateAudioOnlyCaptureAndPassthroughTopology
(
	std::shared_ptr<MediaSource> mediaSource,
	const std::wstring& fileToWrite,
	CComPtr<IMFActivate> audioRendererDevice
)
{
	CreateAudioOnlyCaptureTopology(mediaSource, fileToWrite);
	OnERR_return(GetLastHRESULT());
	InsertTEEAudioRenderer(audioRendererDevice);
}

CComPtr<IMFTopologyNode> TopologyRep::GetFirstNodeTypeFromTopology(MF_TOPOLOGY_TYPE nodeTypeToFind)
{
	WORD nodeCount = 0;
	OnERR_return_NULL(mTopology->GetNodeCount(&nodeCount));
	for (int i = 0; i < nodeCount; i++)
	{
		CComPtr<IMFTopologyNode> node;
		OnERR_return_NULL(mTopology->GetNode(i, &node));
		MF_TOPOLOGY_TYPE nodeType = MF_TOPOLOGY_MAX;
		OnERR_return_NULL(node->GetNodeType(&nodeType));
		if (nodeType == nodeTypeToFind)
		{
			return node;
		}
	}
	return NULL;
}
CComPtr<IMFTopologyNode> TopologyRep::GetSourceNodeFromTopology()
{
	return GetFirstNodeTypeFromTopology(MF_TOPOLOGY_SOURCESTREAM_NODE);
}

CComPtr<IMFTopologyNode> TopologyRep::GetTransformNodeFromTopology()
{
	return GetFirstNodeTypeFromTopology(MF_TOPOLOGY_TRANSFORM_NODE);
}

CComPtr<IMFTopologyNode> TopologyRep::GetOutputNodeFromTopology()
{
	return GetFirstNodeTypeFromTopology(MF_TOPOLOGY_OUTPUT_NODE);
}

void TopologyRep::InsertTEEAudioRenderer(CComPtr<IMFActivate> audioRendererDevice)
{
	CComPtr<IMFTopologyNode> sourceNode = GetSourceNodeFromTopology();
	OnERR_return(GetLastHRESULT());

	CComPtr<IMFTopologyNode> transformNode = GetTransformNodeFromTopology();
	OnERR_return(GetLastHRESULT());

	CComPtr<IMFTopologyNode> outputNode = GetOutputNodeFromTopology();
	OnERR_return(GetLastHRESULT());

	std::shared_ptr<TopologyNode> teeNode = CreateTeeNode();
	OnERR_return(mTopology->AddNode(teeNode->GetNode()));

	std::shared_ptr<TopologyNode> audioRendererNode = CreateRendererNode(audioRendererDevice);
	OnERR_return(mTopology->AddNode(audioRendererNode->GetNode()));

	OnERR_return(AddAndConnect2Nodes(sourceNode, teeNode->GetNode(), 0));
	OnERR_return(AddAndConnect2Nodes(teeNode->GetNode(), transformNode, 0));
	OnERR_return(AddAndConnect2Nodes(transformNode, outputNode, 0));
	OnERR_return(AddAndConnect2Nodes(teeNode->GetNode(), audioRendererNode->GetNode(), 1));
}