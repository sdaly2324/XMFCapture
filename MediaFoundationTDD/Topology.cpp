#include "Topology.h"
#include "TopologyNode.h"
#include "MediaSource.h"
#include "PresentationDescriptor.h"
#include "MFUtils.h"
#include "TranscodeProfileFactory.h"
#include "MediaTypeFactory.h"
#include "FileSink.h"

#include <mfapi.h>
#include <mfidl.h>
#include <strmif.h>
#include <codecapi.h>
#include <memory>
#include <vector>
#include <wmcodecdsp.h>

class TopologyRep : public MFUtils
{
public:
	TopologyRep();
	TopologyRep(CComPtr<IMFMediaEvent> mediaEvent);
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
	void CreateVideoOnlyCaptureAndPassthroughTopology
	(
		std::shared_ptr<MediaSource> mediaSource,
		const std::wstring& fileToWrite,
		CComPtr<IMFActivate> videoRendererDevice
	);
	void CreateVideoAndAudioCaptureAndPassthroughTopology
	(
		std::shared_ptr<MediaSource> videoMediaSource,
		std::shared_ptr<MediaSource> audioMediaSource,
		std::shared_ptr<MediaSource> aggregateMediaSource,
		const std::wstring& fileToWrite,
		CComPtr<IMFActivate> videoRendererDevice,
		CComPtr<IMFActivate> audioRendererDevice
	);

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
	void DumpTopology(CComPtr<IMFTopology> topology);

private:
	void DumpStreamDescriptor(CComPtr<IMFStreamDescriptor> streamDescriptor, std::wstring name);
	void DumpPresentationDescriptor(CComPtr<IMFTopologyNode> node, CComPtr<IMFPresentationDescriptor> presentationDescriptor, std::wstring name);
	std::shared_ptr<TopologyNode>		CreateAudioEncoderTransformNode(CComPtr<IMFMediaType> inputType);
	std::shared_ptr<TopologyNode>		CreateVideoEncoderTransformNode(CComPtr<IMFMediaType> inputType, CComPtr<IMFMediaType> outputType);
	CComPtr<IMFTransform>		GetEncoder(GUID guidMajorType, GUID guidSubtype, GUID mft_category, std::wstring contains1, std::wstring contains2);
	CComPtr<IMFTransform>		GetVideoEncoder();
	CComPtr<IMFTransform>		GetAudioEncoder();
	CComPtr<IMFTopologyNode>	GetFirstNodeTypeFromTopology(MF_TOPOLOGY_TYPE nodeType);
	CComPtr<IMFTopologyNode>	GetSourceNodeFromTopology();
	CComPtr<IMFTopologyNode>	GetTransformNodeFromTopology();
	CComPtr<IMFTopologyNode>	GetOutputNodeFromTopology();

	void											UpdateSourceNodeMediaTypes(CComPtr<IMFTopologyNode> node);
	bool											IsNodeTypeSource(CComPtr<IMFTopologyNode> node);
	CComPtr<IMFTopology>							ResolveMultiSourceTopology(CComPtr<IMFTopology> topology);
	void											BindOutputNode(CComPtr<IMFTopologyNode> node);
	void											BindOutputNodes(CComPtr<IMFTopology> topology);
	void											InspectNodeConections(CComPtr<IMFTopology> topology);
	std::shared_ptr<TopologyNode>					CreateTeeNode(std::wstring name);
	std::shared_ptr<TopologyNode>					CreateColorConverterNode(CComPtr<IMFMediaType> inputType, CComPtr<IMFMediaType> outputType);
	std::shared_ptr<TopologyNode>					CreateAudioConverterNodeFloatToPCM(CComPtr<IMFMediaType> inputType);
	std::shared_ptr<TopologyNode>					CreateSinkNode(std::shared_ptr<FileSink> mediaSink);
	std::shared_ptr<TopologyNode>					CreateRendererNode(std::wstring name, CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice);
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
		std::wstring name,
		CComPtr<IMFMediaSource> mediaSource,
		CComPtr<IMFPresentationDescriptor> presentationDescriptor,
		CComPtr<IMFStreamDescriptor> streamDescriptor,
		CComPtr<IMFActivate> renderer
	);
	bool											NodeExists(CComPtr<IMFTopologyNode> node);
	HRESULT											AddAndConnect2Nodes(CComPtr<IMFTopologyNode> inputNode, DWORD outputIndexOfTheInputNode, CComPtr<IMFTopologyNode> outputNode, DWORD inputIndexOfTheOutputNode);
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

Topology::Topology(CComPtr<IMFMediaEvent> mediaEvent)
{
	m_pRep = std::unique_ptr<TopologyRep>(new TopologyRep(mediaEvent));
}
TopologyRep::TopologyRep(CComPtr<IMFMediaEvent> mediaEvent)
{
	PROPVARIANT var;
	PropVariantInit(&var);
	HRESULT hr = mediaEvent->GetValue(&var);
	hr = var.punkVal->QueryInterface(__uuidof(IMFTopology), (void**)&mTopology);
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
	std::wstring name,
	CComPtr<IMFMediaSource> mediaSource,
	CComPtr<IMFPresentationDescriptor> presentationDescriptor,
	CComPtr<IMFStreamDescriptor> streamDescriptor,
	CComPtr<IMFActivate> renderer
)
{
	std::shared_ptr<TopologyNode> mediaSourceNode(new TopologyNode(name, mediaSource, presentationDescriptor, streamDescriptor, renderer));
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
		return CreateSourceNode(L"Audio Source", mediaSource, presentationDescriptor->GetPresentationDescriptor(), audioStreamDescriptor, renderer);
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
		return CreateSourceNode(L"Video Source", mediaSource, presentationDescriptor->GetPresentationDescriptor(), videoStreamDescriptor, renderer);
	}
	return NULL;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateTeeNode(std::wstring name)
{
	std::shared_ptr<TopologyNode> teeNode(new TopologyNode(name));
	if (teeNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return teeNode;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateRendererNode(std::wstring name, CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice)
{
	std::shared_ptr<TopologyNode> rendererNode(new TopologyNode(name, prefMediaType, renderDevice));
	if (rendererNode->GetLastHRESULT() != S_OK)
	{
		return NULL;
	}
	return rendererNode;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateSinkNode(std::shared_ptr<FileSink> mediaSink)
{
	std::shared_ptr<TopologyNode> sinkNode(new TopologyNode(L"File Sink", mediaSink));
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
		CComPtr<IMFTopologyNode> rendererNode = sourceNode->GetRendererNode();
		if (!rendererNode)
		{
			SetLastHR_Fail();
			return;
		}
		OnERR_return(AddAndConnect2Nodes(sourceNode->GetNode(), 0, sinkNode, 0));
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
		OnERR_return(AddAndConnect2Nodes(sourceNode->GetNode(), 0, rendererNode, 0));
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
HRESULT TopologyRep::AddAndConnect2Nodes(CComPtr<IMFTopologyNode> inputNode, DWORD outputIndexOfTheInputNode, CComPtr<IMFTopologyNode> outputNode, DWORD inputIndexOfTheOutputNode)
{
	if (!NodeExists(inputNode))
	{
		OnERR_return_HR(mTopology->AddNode(inputNode));
	}
	if (!NodeExists(outputNode))
	{
		OnERR_return_HR(mTopology->AddNode(outputNode));
	}
	OnERR_return_HR(inputNode->ConnectOutput(outputIndexOfTheInputNode, outputNode, 0));
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

void TopologyRep::UpdateSourceNodeMediaTypes(CComPtr<IMFTopologyNode> node)
{
	if (IsNodeTypeSource(node))
	{
		CComPtr<IMFMediaSource> mediaSource = NULL;
		if (IsHRError(node->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**)&mediaSource)))
		{
			return;
		}
		std::unique_ptr<PresentationDescriptor> presentationDescriptor(new PresentationDescriptor(mediaSource));
		if (presentationDescriptor->GetFirstAudioStreamDescriptor() && LastHR_OK())
		{
			mHaveAudioSourceNode = true;
		}
		if (presentationDescriptor->GetFirstVideoStreamDescriptor() && LastHR_OK())
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

void TopologyRep::DumpStreamDescriptor(CComPtr<IMFStreamDescriptor> streamDescriptor, std::wstring name)
{
	CComPtr<IMFMediaTypeHandler> mediaTypeHandler = nullptr;
	OnERR_return(streamDescriptor->GetMediaTypeHandler(&mediaTypeHandler));
	CComPtr<IMFMediaType> mediaType = nullptr;
	OnERR_return(mediaTypeHandler->GetCurrentMediaType(&mediaType));
	DumpAttr(mediaType, L"", name);
	OutputDebugStringW(L"\n");
}
void TopologyRep::DumpPresentationDescriptor(CComPtr<IMFTopologyNode> node, CComPtr<IMFPresentationDescriptor> presentationDescriptor, std::wstring name)
{
	CComPtr<IMFStreamDescriptor> streamDescriptorFromNode = nullptr;
	OnERR_return(node->GetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, IID_IMFStreamDescriptor, (void**)&streamDescriptorFromNode));
	DumpStreamDescriptor(streamDescriptorFromNode, name + L" MF_TOPONODE_STREAM_DESCRIPTOR ");

	DWORD streamDescriptorCount = 0;
	OnERR_return(presentationDescriptor->GetStreamDescriptorCount(&streamDescriptorCount));
	for (DWORD streamDescriptorIndex = 0; streamDescriptorIndex < streamDescriptorCount; streamDescriptorIndex++)
	{
		CComPtr<IMFStreamDescriptor> streamDescriptorFromPresIndex = nullptr;
		BOOL selected = FALSE;
		OnERR_return(presentationDescriptor->GetStreamDescriptorByIndex(streamDescriptorIndex, &selected, &streamDescriptorFromPresIndex));
		DumpStreamDescriptor(streamDescriptorFromPresIndex, name + std::to_wstring(streamDescriptorIndex) + L"<-strDesIndex streamDescriptorFromPresIndex ");
	}
}
CComPtr<IMFTopology> TopologyRep::ResolveMultiSourceTopology(CComPtr<IMFTopology> topology)
{
	CComPtr<IMFSequencerSource> sequencerSource = NULL;
	OnERR_return_NULL(MFCreateSequencerSource(NULL, &sequencerSource));
	MFSequencerElementId newMFSequencerElementId = 0;
	//DumpTopology(topology);

	CComPtr<IMFCollection> collection = nullptr;
	topology->GetSourceNodeCollection(&collection);
	DWORD outputNodes = 0;
	OnERR_return_NULL(collection->GetElementCount(&outputNodes));
	for (WORD nodeNumber = 0; nodeNumber < outputNodes; nodeNumber++)
	{
		CComPtr<IUnknown> unknown = NULL;
		OnERR_return_NULL(collection->GetElement(nodeNumber, &unknown));
		CComPtr<IMFTopologyNode> node = NULL;
		OnERR_return_NULL(unknown->QueryInterface(IID_IMFTopologyNode, (void**)&node));

		CComPtr<IMFMediaSource> mediaSource = nullptr;
		OnERR_return_NULL(node->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**)&mediaSource));
		CComPtr<IMFPresentationDescriptor> presentationDescriptorFromSource = nullptr;
		OnERR_return_NULL(mediaSource->CreatePresentationDescriptor(&presentationDescriptorFromSource));
		CComPtr<IMFPresentationDescriptor> presentationDescriptorFromNode = nullptr;
		OnERR_return_NULL(node->GetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, IID_IMFPresentationDescriptor, (void**)&presentationDescriptorFromNode));

		DumpPresentationDescriptor(node, presentationDescriptorFromNode, std::to_wstring(nodeNumber) + L"<-SourceNodeNumber MF_TOPONODE_PRESENTATION_DESCRIPTOR ");
		DumpPresentationDescriptor(node, presentationDescriptorFromSource, std::to_wstring(nodeNumber) + L"<-SourceNodeNumber presentationDescriptorFromSource ");
	}
		
	TOPOID topoID = 0;
	OnERR_return_NULL(topology->GetTopologyID(&topoID));
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
	//OnERR_return(mTopology->SetUINT32(MF_TOPOLOGY_HARDWARE_MODE, MFTOPOLOGY_HWMODE_USE_HARDWARE));
	//OnERR_return(mTopology->SetUINT32(MF_TOPOLOGY_DXVA_MODE, MFTOPOLOGY_DXVA_FULL));
	OnERR_return(mediaSession->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, mTopology));
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

void Topology::CreateVideoAndAudioCaptureAndPassthroughTopology
(
	std::shared_ptr<MediaSource> videoMediaSource,
	std::shared_ptr<MediaSource> audioMediaSource,
	std::shared_ptr<MediaSource> aggregateMediaSource,
	const std::wstring& fileToWrite,
	CComPtr<IMFActivate> videoRendererDevice,
	CComPtr<IMFActivate> audioRendererDevice
)
{
	m_pRep->CreateVideoAndAudioCaptureAndPassthroughTopology(videoMediaSource, audioMediaSource, aggregateMediaSource, fileToWrite, videoRendererDevice, audioRendererDevice);
}
void TopologyRep::CreateVideoAndAudioCaptureAndPassthroughTopology
(
	std::shared_ptr<MediaSource> videoMediaSource,
	std::shared_ptr<MediaSource> audioMediaSource,
	std::shared_ptr<MediaSource> aggregateMediaSource,
	const std::wstring& fileToWrite,
	CComPtr<IMFActivate> videoRendererDevice,
	CComPtr<IMFActivate> audioRendererDevice
)
{
	if (!mTopology)
	{
		OnERR_return(MFCreateTopology(&mTopology));
	}

	if (aggregateMediaSource)
	{
		aggregateMediaSource->SetCurrentMediaTypes();
		videoMediaSource = aggregateMediaSource;
		audioMediaSource = aggregateMediaSource;
	}
	else
	{
		if (videoMediaSource)
		{
			videoMediaSource->SetCurrentMediaTypes();
		}
		if (audioMediaSource)
		{
			audioMediaSource->SetCurrentMediaTypes();
		}
	}

	auto fileSink = std::make_shared<FileSink>(fileToWrite.c_str(), videoMediaSource, audioMediaSource);
	auto fileNode = std::make_unique<TopologyNode>(L"File Sink", fileSink);

	MediaTypeFactory mediaTypeFactory;
	bool hasVideo = videoMediaSource && videoMediaSource->GetVideoMediaType() ? true : false;
	bool hasAudio = audioMediaSource && audioMediaSource->GetAudioMediaType() ? true : false;
	
	if (hasVideo)
	{
		std::shared_ptr<PresentationDescriptor> videoMediaSourcePresentationDescriptor(new PresentationDescriptor(videoMediaSource->GetMediaSource()));

		std::shared_ptr<TopologyNode> videoSourceNode = CreateVideoSourceNode(videoMediaSource->GetMediaSource(), videoMediaSourcePresentationDescriptor, videoRendererDevice);
		std::shared_ptr<TopologyNode> videoRendererNode = CreateRendererNode(L"Video Renderer", videoSourceNode->GetOutputPrefType(), videoRendererDevice);

		//std::shared_ptr<TopologyNode> colorConverterNode = CreateColorConverterNode(videoSourceNode->GetOutputPrefType(), videoRendererNode->GetInputPrefType());

		CComPtr<IMFAttributes> videoSourceAttrs = videoMediaSource->GetVideoMediaType();
		std::shared_ptr<TopologyNode> videoEncoderNode = CreateVideoEncoderTransformNode(videoRendererNode->GetInputPrefType(), mediaTypeFactory.CreateVideoEncodingMediaType(videoSourceAttrs));
		std::shared_ptr<TopologyNode> videoTeeNode = CreateTeeNode(L"Video TEE");

		OnERR_return(AddAndConnect2Nodes(videoSourceNode->GetNode(), 0, videoTeeNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(videoTeeNode->GetNode(), 0, videoRendererNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(videoTeeNode->GetNode(), 1, videoEncoderNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(videoEncoderNode->GetNode(), 0, fileNode->GetNode(), 0));
	}
	if (hasAudio)
	{
		std::shared_ptr<PresentationDescriptor> audioMediaSourcePresentationDescriptor(new PresentationDescriptor(audioMediaSource->GetMediaSource()));

		std::shared_ptr<TopologyNode> audioSourceNode = CreateAudioSourceNode(audioMediaSource->GetMediaSource(), audioMediaSourcePresentationDescriptor, audioRendererDevice);
		std::shared_ptr<TopologyNode> audioRendererNode = CreateRendererNode(L"Audio Renderer", audioSourceNode->GetOutputPrefType(), audioRendererDevice);

		//std::shared_ptr<TopologyNode> audioFloatToPCMNode = CreateAudioConverterNodeFloatToPCM(audioSourceNode->GetOutputPrefType());
		//std::shared_ptr<TopologyNode> audioEncoderNode = CreateAudioEncoderTransformNode(audioFloatToPCMNode->GetOutputPrefType());
		
		std::shared_ptr<TopologyNode> audioEncoderNode = CreateAudioEncoderTransformNode(mediaTypeFactory.CreateAudioInputMediaType());
		std::shared_ptr<TopologyNode> audioTeeNode = CreateTeeNode(L"Audio Tee");

		OnERR_return(AddAndConnect2Nodes(audioSourceNode->GetNode(), 0, audioTeeNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(audioTeeNode->GetNode(), 0, audioRendererNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(audioTeeNode->GetNode(), 1, audioEncoderNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(audioEncoderNode->GetNode(), 0, fileNode->GetNode(), 1));

	}
}

std::shared_ptr<TopologyNode> TopologyRep::CreateAudioEncoderTransformNode(CComPtr<IMFMediaType> inputType)
{
	CComPtr<IMFTransform> transform = GetAudioEncoder();
	if (!transform)
	{
		SetLastHR_Fail();
		return nullptr;
	}
	CComPtr<IMFMediaType> outputMediaType = nullptr;
	DWORD formatIndexWeWantForOutput = 43;
	//	Audio AAC Output(43) ATTR 000 MF_MT_AUDIO_AVG_BYTES_PER_SECOND                         24000
	//	Audio AAC Output(43) ATTR 001 MF_MT_AVG_BITRATE                                        192000
	//	Audio AAC Output(43) ATTR 002 MF_MT_AUDIO_BLOCK_ALIGNMENT                              1
	//	Audio AAC Output(43) ATTR 003 MF_MT_AUDIO_NUM_CHANNELS                                 2
	//	Audio AAC Output(43) ATTR 004 MF_MT_COMPRESSED                                         1
	//	Audio AAC Output(43) ATTR 005 MF_MT_MAJOR_TYPE                                         MFMediaType_Audio
	//	Audio AAC Output(43) ATTR 006 MF_MT_AUDIO_SAMPLES_PER_SECOND                           48000
	//	Audio AAC Output(43) ATTR 007 MF_MT_AM_FORMAT_TYPE                                     FORMAT_WaveFormatEx
	//	Audio AAC Output(43) ATTR 008 MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION                 41
	//	Audio AAC Output(43) ATTR 009 MF_MT_AUDIO_PREFER_WAVEFORMATEX                          1
	//	Audio AAC Output(43) ATTR 010 MF_MT_USER_DATA                                          BLOB
	//	Audio AAC Output(43) ATTR 011 MF_MT_FIXED_SIZE_SAMPLES                                 0
	//	Audio AAC Output(43) ATTR 012 MF_MT_AAC_PAYLOAD_TYPE                                   1
	//	Audio AAC Output(43) ATTR 013 MF_MT_AUDIO_BITS_PER_SAMPLE                              16
	//	Audio AAC Output(43) ATTR 014 MF_MT_SUBTYPE                                            MFAudioFormat_AAC
	OnERR_return_NULL(transform->GetOutputAvailableType(0, formatIndexWeWantForOutput, &outputMediaType));
	OnERR_return_NULL(transform->SetOutputType(0, outputMediaType, 0));
	OnERR_return_NULL(transform->SetInputType(0, inputType, 0));

	return std::make_shared<TopologyNode>(L"Audio AAC Encoder", transform);
}

std::shared_ptr<TopologyNode> TopologyRep::CreateVideoEncoderTransformNode(CComPtr<IMFMediaType> inputType, CComPtr<IMFMediaType> outputType)
{
	CComPtr<IMFTransform> transform = GetVideoEncoder();
	if (!transform)
	{
		SetLastHR_Fail();
		return nullptr;
	}
	CComPtr<IMFAttributes> attributes = nullptr;
	OnERR_return_NULL(transform->GetAttributes(&attributes));
	OnERR_return_NULL(attributes->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE));
	OnERR_return_NULL(attributes->SetUINT32(MF_LOW_LATENCY, TRUE));

	// must set out before in or in will fail
	OnERR_return_NULL(transform->SetOutputType(0, outputType, 0));
	OnERR_return_NULL(transform->SetInputType(0, inputType, 0));

	return std::make_shared<TopologyNode>(L"Video H264 Encoder", transform);
}

void Topology::CreateVideoOnlyCaptureAndPassthroughTopology
(
	std::shared_ptr<MediaSource> mediaSource,
	const std::wstring& fileToWrite,
	CComPtr<IMFActivate> videoRendererDevice
)
{
	m_pRep->CreateVideoOnlyCaptureAndPassthroughTopology(mediaSource, fileToWrite, videoRendererDevice);
}
void TopologyRep::CreateVideoOnlyCaptureAndPassthroughTopology
(
	std::shared_ptr<MediaSource> mediaSource,
	const std::wstring& fileToWrite,
	CComPtr<IMFActivate> videoRendererDevice
)
{
	if (!mTopology)
	{
		OnERR_return(MFCreateTopology(&mTopology));
	}
	mediaSource->SetCurrentMediaTypes();

	MediaTypeFactory mediaTypeFactory;
	std::shared_ptr<PresentationDescriptor> mediaSourcePresentationDescriptor(new PresentationDescriptor(mediaSource->GetMediaSource()));
	std::shared_ptr<TopologyNode> videoSourceNode = CreateVideoSourceNode(mediaSource->GetMediaSource(), mediaSourcePresentationDescriptor, videoRendererDevice);
	std::shared_ptr<TopologyNode> videoRendererNode = CreateRendererNode(L"Video Renderer", videoSourceNode->GetOutputPrefType(), videoRendererDevice);
	std::shared_ptr<TopologyNode> teeNode = CreateTeeNode(L"Video TEE");
	std::shared_ptr<TopologyNode> colorConverterNode = CreateColorConverterNode(videoSourceNode->GetOutputPrefType(), videoRendererNode->GetInputPrefType());

	// encoder
	CComPtr<IMFAttributes> sourceAttrs = mediaSource->GetVideoMediaType();
	std::shared_ptr<TopologyNode> encoderNode = CreateVideoEncoderTransformNode(videoRendererNode->GetInputPrefType(), mediaTypeFactory.CreateVideoEncodingMediaType(sourceAttrs));
	auto fileSink = std::make_shared<FileSink>(fileToWrite.c_str(), mediaSource , nullptr);
	auto fileNode = std::make_unique<TopologyNode>(L"File Sink", fileSink);

	OnERR_return(AddAndConnect2Nodes(videoSourceNode->GetNode(), 0, colorConverterNode->GetNode(), 0));
	OnERR_return(AddAndConnect2Nodes(colorConverterNode->GetNode(), 0, teeNode->GetNode(), 0));
	OnERR_return(AddAndConnect2Nodes(teeNode->GetNode(), 0, videoRendererNode->GetNode(), 0));
	OnERR_return(AddAndConnect2Nodes(teeNode->GetNode(), 1, encoderNode->GetNode(), 0));
	OnERR_return(AddAndConnect2Nodes(encoderNode->GetNode(), 0, fileNode->GetNode(), 0));
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

std::shared_ptr<TopologyNode> TopologyRep::CreateAudioConverterNodeFloatToPCM(CComPtr<IMFMediaType> inputType)
{
	CComPtr<IMFTransform> transform = nullptr;
	OnERR_return_NULL(CoCreateInstance(CLSID_CResamplerMediaObject, nullptr, CLSCTX_INPROC, IID_IMFTransform, (void**)&transform));
	OnERR_return_NULL(transform->SetInputType(0, inputType, 0));

	CComPtr<IMFMediaType> outputMediaType = nullptr;
	DWORD formatIndexWeWantForOutput = 3;
	//	CLSID_CResamplerMediaObject GetOutputAvailableType(3) ATTR 000 MF_MT_AUDIO_AVG_BYTES_PER_SECOND                         192000
	//	CLSID_CResamplerMediaObject GetOutputAvailableType(3) ATTR 001 MF_MT_AUDIO_BLOCK_ALIGNMENT                              4
	//	CLSID_CResamplerMediaObject GetOutputAvailableType(3) ATTR 002 MF_MT_AUDIO_NUM_CHANNELS                                 2
	//	CLSID_CResamplerMediaObject GetOutputAvailableType(3) ATTR 003 MF_MT_MAJOR_TYPE                                         MFMediaType_Audio
	//	CLSID_CResamplerMediaObject GetOutputAvailableType(3) ATTR 004 MF_MT_AUDIO_SAMPLES_PER_SECOND                           48000
	//	CLSID_CResamplerMediaObject GetOutputAvailableType(3) ATTR 005 MF_MT_AUDIO_PREFER_WAVEFORMATEX                          1
	//	CLSID_CResamplerMediaObject GetOutputAvailableType(3) ATTR 006 MF_MT_ALL_SAMPLES_INDEPENDENT                            1
	//	CLSID_CResamplerMediaObject GetOutputAvailableType(3) ATTR 007 MF_MT_AUDIO_BITS_PER_SAMPLE                              16
	//	CLSID_CResamplerMediaObject GetOutputAvailableType(3) ATTR 008 MF_MT_SUBTYPE                                            MFAudioFormat_PCM
	OnERR_return_NULL(transform->GetOutputAvailableType(0, formatIndexWeWantForOutput, &outputMediaType));
	OnERR_return_NULL(transform->SetOutputType(0, outputMediaType, 0));

	std::shared_ptr<TopologyNode> retVal(new TopologyNode(L"Audio Float to PCM", transform));
	return retVal;
}

std::shared_ptr<TopologyNode> TopologyRep::CreateColorConverterNode(CComPtr<IMFMediaType> inputType, CComPtr<IMFMediaType> outputType)
{
	CComPtr<IMFTransform> transform = nullptr;
	OnERR_return_NULL(CoCreateInstance(CLSID_CColorConvertDMO, nullptr, CLSCTX_INPROC, IID_IMFTransform, (void**)&transform));
	OnERR_return_NULL(transform->SetInputType(0, inputType, 0));
	OnERR_return_NULL(transform->SetOutputType(0, outputType, 0));

	std::shared_ptr<TopologyNode> retVal(new TopologyNode(L"Color Converter", transform));
	return retVal;
}

void Topology::DumpTopology(CComPtr<IMFTopology> topology)
{
	m_pRep->DumpTopology(topology);
}
void TopologyRep::DumpTopology(CComPtr<IMFTopology> topology)
{
	if (!topology)
	{
		topology = mTopology;
	}
	WORD nodeCount = 0;
	OnERR_return(topology->GetNodeCount(&nodeCount));
	for (int i = 0; i < nodeCount; i++)
	{
		OutputDebugStringW(L"*******************************************************\n");
		CComPtr<IMFTopologyNode> node;
		OnERR_return(topology->GetNode(i, &node));
		MF_TOPOLOGY_TYPE nodeType = MF_TOPOLOGY_MAX;
		OnERR_return(node->GetNodeType(&nodeType));
		if (nodeType == MF_TOPOLOGY_SOURCESTREAM_NODE)
		{
			OutputDebugStringW(L"MF_TOPOLOGY_SOURCESTREAM_NODE\n\n");
			auto srcNode = std::make_unique<TopologyNode>(L"MF_TOPOLOGY_SOURCESTREAM_NODE", node);
			DumpAttr(srcNode->GetOutputPrefType(), std::to_wstring(i), L"srcNode");
		}
		else if (nodeType == MF_TOPOLOGY_OUTPUT_NODE)
		{
			OutputDebugStringW(L"MF_TOPOLOGY_OUTPUT_NODE\n\n");
			auto outputNode = std::make_unique<TopologyNode>(L"MF_TOPOLOGY_OUTPUT_NODE", node);
			DumpAttr(outputNode->GetInputPrefType(), std::to_wstring(i), L"outputNode");
		}
		else if (nodeType == MF_TOPOLOGY_TRANSFORM_NODE)
		{
			OutputDebugStringW(L"MF_TOPOLOGY_TRANSFORM_NODE\n\n");
			auto transformNode = std::make_unique<TopologyNode>(L"MF_TOPOLOGY_TRANSFORM_NODE", node);
			DumpAttr(transformNode->GetInputPrefType(), std::to_wstring(i), L"transformNode IN");
			OutputDebugStringW(L"\n");
			DumpAttr(transformNode->GetOutputPrefType(), std::to_wstring(i), L"transformNode OUT");
		}
		else if (nodeType == MF_TOPOLOGY_TEE_NODE)
		{
			OutputDebugStringW(L"MF_TOPOLOGY_TEE_NODE\n\n");
		}

		GUID guid;
		HRESULT hr = node->GetGUID(MF_TOPONODE_ERROR_MAJORTYPE, &guid);
		if (SUCCEEDED(hr))
		{
			OutputDebugStringW(L"MF_TOPONODE_ERROR_MAJORTYPE\n");
		}
		hr = node->GetGUID(MF_TOPONODE_ERROR_SUBTYPE, &guid);
		if (SUCCEEDED(hr))
		{
			OutputDebugStringW(L"MF_TOPONODE_ERROR_SUBTYPE\n");
		}
		UINT32 value32 = 0;
		hr = node->GetUINT32(MF_TOPONODE_ERRORCODE, &value32);
		if (SUCCEEDED(hr))
		{
			OutputDebugStringW(L"MF_TOPONODE_ERRORCODE\n");
		}
		OutputDebugStringW(L"\n");
	}
}

CComPtr<IMFTransform> TopologyRep::GetEncoder(GUID guidMajorType, GUID guidSubtype, GUID mft_category, std::wstring contains1, std::wstring contains2)
{
	CComPtr<IMFTransform> retval = nullptr;

	IMFActivate** codecList = 0;
	UINT32 count = 0;
	MFT_REGISTER_TYPE_INFO typeInfo;
	typeInfo.guidMajorType = guidMajorType;
	typeInfo.guidSubtype = guidSubtype;

	OnERR_return_NULL(MFTEnumEx
	(
		mft_category,
		MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_HARDWARE | MFT_ENUM_FLAG_SORTANDFILTER,
		0,
		&typeInfo,
		&codecList,
		&count
	));
	bool foundEncoder = false;
	UINT32 encoderIndex = 0;
	for (; encoderIndex < count && foundEncoder == false; encoderIndex++)
	{
		LPWSTR name = 0;
		OnERR_return_NULL(codecList[encoderIndex]->GetAllocatedString(MFT_FRIENDLY_NAME_Attribute, &name, 0));
		std::wstring namew(name);
		if (namew.find(contains1) != std::string::npos && namew.find(contains2) != std::string::npos)
		{
			foundEncoder = true;
		}
		CoTaskMemFree(name);
	}
	if (foundEncoder)
	{
		codecList[encoderIndex - 1]->ActivateObject(IID_PPV_ARGS(&retval));
	}
	for (UINT32 i = 0; i < count; i++)
	{
		codecList[i]->Release();
	}

	return retval;
}
CComPtr<IMFTransform> TopologyRep::GetAudioEncoder()
{
	return GetEncoder(MFMediaType_Audio, MFAudioFormat_AAC, MFT_CATEGORY_AUDIO_ENCODER, L"Audio", L"AAC");
}

CComPtr<IMFTransform> TopologyRep::GetVideoEncoder()
{
	return GetEncoder(MFMediaType_Video, MFVideoFormat_H264, MFT_CATEGORY_VIDEO_ENCODER, L"H264", L"Encoder");
}