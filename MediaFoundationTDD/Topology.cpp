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
#include <algorithm>
#include <wmcodecdsp.h>

class TopologyRep : public MFUtils
{
public:
	TopologyRep();
	TopologyRep(CComPtr<IMFMediaEvent> mediaEvent);
	~TopologyRep();

	HRESULT GetLastHRESULT();

	void CreateTopology
	(
		std::shared_ptr<MediaSource> aggregateMediaSource,
		const std::wstring& fileToWrite,
		CComPtr<IMFActivate> videoRendererDevice,
		CComPtr<IMFActivate> audioRendererDevice
	);

	void ResolveTopology();
	void SetTopology(CComPtr<IMFMediaSession> mediaSession);

	CComPtr<IMFTopology> GetTopology();
	void DumpTopology(CComPtr<IMFTopology> topology);

private:
	void DumpNode(CComPtr<IMFTopologyNode> node, int index);
	void DumpSourceNodes(CComPtr<IMFTopology> topology);
	void DumpStreamDescriptor(CComPtr<IMFStreamDescriptor> streamDescriptor, std::wstring name);
	void DumpPresentationDescriptor(CComPtr<IMFTopologyNode> node, CComPtr<IMFPresentationDescriptor> presentationDescriptor, std::wstring name);
	std::shared_ptr<TopologyNode>		CreateAudioEncoderTransformNode(CComPtr<IMFMediaType> inputType);
	std::shared_ptr<TopologyNode>		CreateVideoEncoderTransformNode(CComPtr<IMFMediaType> inputType, CComPtr<IMFMediaType> outputType);
	std::shared_ptr<TopologyNode>		CreateVideoSinkNode(std::shared_ptr<FileSink> aggregateSink);
	std::shared_ptr<TopologyNode>		CreateAudioSinkNode(std::shared_ptr<FileSink> aggregateSink);
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
	OnERR_return_HR(inputNode->ConnectOutput(outputIndexOfTheInputNode, outputNode, inputIndexOfTheOutputNode));
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

void TopologyRep::DumpSourceNodes(CComPtr<IMFTopology> topology)
{
		CComPtr<IMFCollection> collection = nullptr;
	topology->GetSourceNodeCollection(&collection);
	DWORD outputNodes = 0;
	OnERR_return(collection->GetElementCount(&outputNodes));
	for (WORD nodeNumber = 0; nodeNumber < outputNodes; nodeNumber++)
	{
		CComPtr<IUnknown> unknown = NULL;
		OnERR_return(collection->GetElement(nodeNumber, &unknown));
		CComPtr<IMFTopologyNode> node = NULL;
		OnERR_return(unknown->QueryInterface(IID_IMFTopologyNode, (void**)&node));

		CComPtr<IMFMediaSource> mediaSource = nullptr;
		OnERR_return(node->GetUnknown(MF_TOPONODE_SOURCE, IID_IMFMediaSource, (void**)&mediaSource));
		CComPtr<IMFPresentationDescriptor> presentationDescriptorFromSource = nullptr;
		OnERR_return(mediaSource->CreatePresentationDescriptor(&presentationDescriptorFromSource));
		CComPtr<IMFPresentationDescriptor> presentationDescriptorFromNode = nullptr;
		OnERR_return(node->GetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, IID_IMFPresentationDescriptor, (void**)&presentationDescriptorFromNode));

		DumpPresentationDescriptor(node, presentationDescriptorFromNode, std::to_wstring(nodeNumber) + L"<-SourceNodeNumber MF_TOPONODE_PRESENTATION_DESCRIPTOR ");
		DumpPresentationDescriptor(node, presentationDescriptorFromSource, std::to_wstring(nodeNumber) + L"<-SourceNodeNumber presentationDescriptorFromSource ");
	}
}
CComPtr<IMFTopology> TopologyRep::ResolveMultiSourceTopology(CComPtr<IMFTopology> topology)
{
	CComPtr<IMFSequencerSource> sequencerSource = NULL;
	OnERR_return_NULL(MFCreateSequencerSource(NULL, &sequencerSource));
	MFSequencerElementId newMFSequencerElementId = 0;
	//DumpSourceNodes(topology);
	TOPOID topoID = 0;
	OnERR_return_NULL(topology->GetTopologyID(&topoID));
	//DumpTopology(mTopology);
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
			else
			{
				DumpNode(node, inputNumber);
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

void Topology::CreateTopology
(
	std::shared_ptr<MediaSource> aggregateMediaSource,
	const std::wstring& fileToWrite,
	CComPtr<IMFActivate> videoRendererDevice,
	CComPtr<IMFActivate> audioRendererDevice
)
{
	m_pRep->CreateTopology(aggregateMediaSource, fileToWrite, videoRendererDevice, audioRendererDevice);
}
void TopologyRep::CreateTopology
(
	std::shared_ptr<MediaSource> aggregateMediaSource,
	const std::wstring& fileToWrite,
	CComPtr<IMFActivate> videoRendererDevice,
	CComPtr<IMFActivate> audioRendererDevice
)
{
	if (!aggregateMediaSource)
	{
		SetLastHR_Fail();
		return;
	}
	if (!mTopology)
	{
		OnERR_return(MFCreateTopology(&mTopology));
	}

	aggregateMediaSource->SetCurrentMediaTypes(); // need to figure out how to set these depending on what is plugged in
	auto aggregateSink = std::make_shared<FileSink>(fileToWrite.c_str(), aggregateMediaSource);

	std::shared_ptr<TopologyNode> videoSinkNode = nullptr;
	std::shared_ptr<TopologyNode> audioSinkNode = nullptr;
	if (aggregateMediaSource->GetVideoMediaType())
	{
		videoSinkNode = CreateVideoSinkNode(aggregateSink);
	}
	if (aggregateMediaSource->GetAudioMediaType())
	{
		audioSinkNode = CreateAudioSinkNode(aggregateSink);
	}

	MediaTypeFactory mediaTypeFactory;
	if (videoSinkNode)
	{
		std::shared_ptr<PresentationDescriptor> videoMediaSourcePresentationDescriptor(new PresentationDescriptor(aggregateMediaSource->GetMediaSource()));

		std::shared_ptr<TopologyNode> videoSourceNode = CreateVideoSourceNode(aggregateMediaSource->GetMediaSource(), videoMediaSourcePresentationDescriptor, videoRendererDevice);
		std::shared_ptr<TopologyNode> videoRendererNode = CreateRendererNode(L"Video Renderer", videoSourceNode->GetOutputPrefType(), videoRendererDevice);

		//std::shared_ptr<TopologyNode> colorConverterNode = CreateColorConverterNode(videoSourceNode->GetOutputPrefType(), videoRendererNode->GetInputPrefType());

		CComPtr<IMFAttributes> videoSourceAttrs = aggregateMediaSource->GetVideoMediaType();
		std::shared_ptr<TopologyNode> videoEncoderNode = CreateVideoEncoderTransformNode(videoRendererNode->GetInputPrefType(), mediaTypeFactory.CreateVideoEncodingMediaType(videoSourceAttrs));
		std::shared_ptr<TopologyNode> videoTeeNode = CreateTeeNode(L"Video TEE");

		// full
		OnERR_return(AddAndConnect2Nodes(videoSourceNode->GetNode(), 0, videoTeeNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(videoTeeNode->GetNode(), 0, videoRendererNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(videoTeeNode->GetNode(), 1, videoEncoderNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(videoEncoderNode->GetNode(), 0, videoSinkNode->GetNode(), 0));

		// pass
		//OnERR_return(AddAndConnect2Nodes(videoSourceNode->GetNode(), 0, videoRendererNode->GetNode(), 0));

		// write
		//OnERR_return(AddAndConnect2Nodes(videoSourceNode->GetNode(), 0, videoEncoderNode->GetNode(), 0));
		//OnERR_return(AddAndConnect2Nodes(videoEncoderNode->GetNode(), 0, videoFileNode->GetNode(), 0));
	}
	if (audioSinkNode)
	{
		std::shared_ptr<PresentationDescriptor> audioMediaSourcePresentationDescriptor(new PresentationDescriptor(aggregateMediaSource->GetMediaSource()));

		std::shared_ptr<TopologyNode> audioSourceNode = CreateAudioSourceNode(aggregateMediaSource->GetMediaSource(), audioMediaSourcePresentationDescriptor, audioRendererDevice);
		std::shared_ptr<TopologyNode> audioRendererNode = CreateRendererNode(L"Audio Renderer", audioSourceNode->GetOutputPrefType(), audioRendererDevice);

		//std::shared_ptr<TopologyNode> audioFloatToPCMNode = CreateAudioConverterNodeFloatToPCM(audioSourceNode->GetOutputPrefType());
		
		std::shared_ptr<TopologyNode> audioEncoderNode = CreateAudioEncoderTransformNode(mediaTypeFactory.CreateAudioInputMediaType());
		std::shared_ptr<TopologyNode> audioTeeNode = CreateTeeNode(L"Audio Tee");

		// full
		OnERR_return(AddAndConnect2Nodes(audioSourceNode->GetNode(), 0, audioTeeNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(audioTeeNode->GetNode(), 0, audioRendererNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(audioTeeNode->GetNode(), 1, audioEncoderNode->GetNode(), 0));
		OnERR_return(AddAndConnect2Nodes(audioEncoderNode->GetNode(), 0, audioSinkNode->GetNode(), 0));

		// pass
		//OnERR_return(AddAndConnect2Nodes(audioSourceNode->GetNode(), 0, audioRendererNode->GetNode(), 0));

		// write
		//OnERR_return(AddAndConnect2Nodes(audioSourceNode->GetNode(), 0, audioEncoderNode->GetNode(), 0));
		//OnERR_return(AddAndConnect2Nodes(audioEncoderNode->GetNode(), 0, audioFileNode->GetNode(), 1));
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
	//CComPtr<IMFMediaType> outputMediaType = nullptr;
	//OnERR_return_NULL(transform->GetOutputAvailableType(0, formatIndexWeWantForOutput, &outputMediaType));

	MediaTypeFactory mediaTypeFactory;
	OnERR_return_NULL(transform->SetOutputType(0, mediaTypeFactory.CreateAudioEncodingMediaType(), 0));
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
		CComPtr<IMFTopologyNode> node;
		OnERR_return(topology->GetNode(i, &node));
		DumpNode(node, i);
	}
}

void TopologyRep::DumpNode(CComPtr<IMFTopologyNode> node, int index)
{
	OutputDebugStringW(L"*******************************************************\n");
	MF_TOPOLOGY_TYPE nodeType = MF_TOPOLOGY_MAX;
	OnERR_return(node->GetNodeType(&nodeType));
	if (nodeType == MF_TOPOLOGY_SOURCESTREAM_NODE)
	{
		OutputDebugStringW(L"MF_TOPOLOGY_SOURCESTREAM_NODE\n\n");
		auto srcNode = std::make_unique<TopologyNode>(L"MF_TOPOLOGY_SOURCESTREAM_NODE", node);
		DumpAttr(srcNode->GetOutputPrefType(), std::to_wstring(index), L"srcNode");
	}
	else if (nodeType == MF_TOPOLOGY_OUTPUT_NODE)
	{
		OutputDebugStringW(L"MF_TOPOLOGY_OUTPUT_NODE\n\n");
		DWORD inputCount = 0;
		node->GetInputCount(&inputCount);
		DWORD outputCount = 0;
		node->GetOutputCount(&outputCount);
		auto outputNode = std::make_unique<TopologyNode>(L"MF_TOPOLOGY_OUTPUT_NODE", node);
		DumpAttr(outputNode->GetInputPrefType(), std::to_wstring(index), L"outputNode ins=" + std::to_wstring(inputCount) + L" outs=" + std::to_wstring(outputCount));
	}
	else if (nodeType == MF_TOPOLOGY_TRANSFORM_NODE)
	{
		OutputDebugStringW(L"MF_TOPOLOGY_TRANSFORM_NODE\n\n");
		auto transformNode = std::make_unique<TopologyNode>(L"MF_TOPOLOGY_TRANSFORM_NODE", node);
		DumpAttr(transformNode->GetInputPrefType(), std::to_wstring(index), L"transformNode IN");
		OutputDebugStringW(L"\n");
		DumpAttr(transformNode->GetOutputPrefType(), std::to_wstring(index), L"transformNode OUT");
	}
	else if (nodeType == MF_TOPOLOGY_TEE_NODE)
	{
		OutputDebugStringW(L"MF_TOPOLOGY_TEE_NODE\n\n");
		auto teeNode = std::make_unique<TopologyNode>(L"MF_TOPOLOGY_TEE_NODE", node);
		DumpAttr(teeNode->GetInputPrefType(), std::to_wstring(index), L"tee IN");
		OutputDebugStringW(L"\n");
		DumpAttr(teeNode->GetOutputPrefType(), std::to_wstring(index), L"tee OUT");
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

std::shared_ptr<TopologyNode> TopologyRep::CreateVideoSinkNode(std::shared_ptr<FileSink> aggregateSink)
{
	return std::make_shared<TopologyNode>(L"Video Stream Sink", aggregateSink->GetVideoStreamSink());
}
std::shared_ptr<TopologyNode> TopologyRep::CreateAudioSinkNode(std::shared_ptr<FileSink> aggregateSink)
{
	return std::make_shared<TopologyNode>(L"Audio Stream Sink", aggregateSink->GetAudioStreamSink());
}