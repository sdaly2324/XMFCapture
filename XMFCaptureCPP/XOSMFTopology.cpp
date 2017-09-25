#include "XOSMFTopology.h"

#include "XOSUtilities.h"
#include "XOSMFAVSourceReader.h"
#include "XOSMFSinkWriter.h"

class XOSMFTopologyRep
{
public:
	XOSMFTopologyRep(XOSMFAVSourceReader* pXOSMFAVSourceReader, bool previewOnly, HWND hwndVideo, XOSMFSinkWriter* pXOSMFSinkWriter);
	~XOSMFTopologyRep();

	HRESULT Stop();

private:
	XOSMFTopologyRep();

	HRESULT AddBranchToPartialTopology(CComPtr<IMFPresentationDescriptor> pPresDescriptor, DWORD nStream, CComPtr<IMFMediaSource> pMFMediaSource, bool previewOnly, HWND hwndVideo);
	HRESULT CreateSourceStreamNode(CComPtr<IMFPresentationDescriptor> pPresDescriptor, CComPtr<IMFStreamDescriptor> pStreamDescriptor, CComPtr<IMFTopologyNode>& pNode, CComPtr<IMFMediaSource> pMFMediaSource);
	HRESULT CreateFileWriterNode(CComPtr<IMFStreamDescriptor> pStreamDescriptor, CComPtr<IMFTopologyNode>& pNode);
	HRESULT CreateOutputNode(CComPtr<IMFStreamDescriptor> pStreamDescriptor, HWND hwndVideo, CComPtr<IMFTopologyNode>& pNode);

	CComPtr<IMFMediaSession>	m_pMFMediaSession;
	CComPtr<IMFTopology>		m_pMFTopology;

	XOSMFAVSourceReader*	m_pXOSMFAVSourceReader;
	XOSMFSinkWriter*		m_pXOSMFSinkWriter;
};

XOSMFTopology::XOSMFTopology(XOSMFAVSourceReader* pXOSMFAVSourceReader, bool previewOnly, HWND hwndVideo, XOSMFSinkWriter* pXOSMFSinkWriter)
{
	m_pRep = new XOSMFTopologyRep(pXOSMFAVSourceReader, previewOnly, hwndVideo, pXOSMFSinkWriter);
}
XOSMFTopologyRep::XOSMFTopologyRep(XOSMFAVSourceReader* pXOSMFAVSourceReader, bool previewOnly, HWND hwndVideo, XOSMFSinkWriter* pXOSMFSinkWriter) :
m_pXOSMFAVSourceReader(pXOSMFAVSourceReader),
m_pXOSMFSinkWriter(pXOSMFSinkWriter),
m_pMFTopology(NULL),
m_pMFMediaSession(NULL)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		hr = MFCreateMediaSession(NULL, &m_pMFMediaSession);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = MFCreateTopology(&m_pMFTopology);
	}

	CComPtr<IMFPresentationDescriptor> pPresDescriptor = NULL;
	if (SUCCEEDED_XOSb(hr) && m_pXOSMFAVSourceReader)
	{
		hr = m_pXOSMFAVSourceReader->GetPresentationDescriptor(pPresDescriptor);
	}

	if (SUCCEEDED_XOSb(hr) && pPresDescriptor)
	{
		DWORD nSourceStreams = 0;
		hr = pPresDescriptor->GetStreamDescriptorCount(&nSourceStreams);

		if (SUCCEEDED_XOSb(hr) && nSourceStreams > 0)
		{
			CComPtr<IMFMediaSource> pMFMediaSource = NULL;
			if (SUCCEEDED_XOSb(hr) && m_pXOSMFAVSourceReader)
			{
				hr = m_pXOSMFAVSourceReader->GetMFMediaSource(pMFMediaSource);
			}

			if (SUCCEEDED_XOSb(hr) && pMFMediaSource)
			{
				for (DWORD x = 0; x < nSourceStreams; x++)
				{
					hr = AddBranchToPartialTopology(pPresDescriptor, x, pMFMediaSource, previewOnly, hwndVideo);

					// if we failed to build a branch for this stream type, then deselect it
					// that will cause the stream to be disabled, and the source will not produce
					// any data for it
					if (FAILED(hr))
					{
						hr = pPresDescriptor->DeselectStream(x);
					}
				}
			}
		}
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = m_pMFMediaSession->SetTopology(NULL, m_pMFTopology);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		hr = m_pMFMediaSession->Start(&GUID_NULL, &varStart);
	}

	SUCCEEDED_XOSv(hr);
}

XOSMFTopology::~XOSMFTopology()
{
	if (m_pRep)
	{
		delete m_pRep;
	}
	m_pRep = NULL;
}
XOSMFTopologyRep::~XOSMFTopologyRep()
{
	if (m_pXOSMFSinkWriter)
	{
		delete m_pXOSMFSinkWriter;
	}
	m_pXOSMFSinkWriter = NULL;

	if (m_pXOSMFAVSourceReader)
	{
		delete m_pXOSMFAVSourceReader;
	}
	m_pXOSMFAVSourceReader = NULL;

	if (m_pMFTopology)
	{
		delete m_pMFTopology;
	}
	m_pMFTopology = NULL;
}

HRESULT XOSMFTopologyRep::AddBranchToPartialTopology(CComPtr<IMFPresentationDescriptor> pPresDescriptor, DWORD nStream, CComPtr<IMFMediaSource> pMFMediaSource, bool /*previewOnly*/, HWND hwndVideo)
{
	if (!pPresDescriptor || !pMFMediaSource)
	{
		return E_INVALIDARG;
	}
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr) && m_pMFTopology)
	{
		// Get the stream descriptor for this stream (information about stream).
		BOOL streamSelected = FALSE;
		CComPtr<IMFStreamDescriptor> pStreamDescriptor = NULL;
		hr = pPresDescriptor->GetStreamDescriptorByIndex(nStream, &streamSelected, &pStreamDescriptor);

		// Create the topology branch only if the stream is selected - IE if the user wants to play it.
		if (streamSelected && SUCCEEDED_XOSb(hr) && pStreamDescriptor)
		{
			// Create a source node for this stream.
			CComPtr<IMFTopologyNode> pSourceNode = NULL;
			hr = CreateSourceStreamNode(pPresDescriptor, pStreamDescriptor, pSourceNode, pMFMediaSource);
			if (SUCCEEDED_XOSb(hr) && pSourceNode)
			{
				// Add the source node to the topology.
				hr = m_pMFTopology->AddNode(pSourceNode);
			}

			// Create the out node for the renderer.
			CComPtr<IMFTopologyNode> pOutPutNode = NULL;
			if (SUCCEEDED_XOSb(hr))
			{
				hr = CreateOutputNode(pStreamDescriptor, hwndVideo, pOutPutNode);
			}
			if (SUCCEEDED_XOSb(hr) && pOutPutNode)
			{
				// Add the out node to the topology.
				hr = m_pMFTopology->AddNode(pOutPutNode);
			}
			if (SUCCEEDED_XOSb(hr))
			{
				// Connect the source node to the out node.  The resolver will find the
				// intermediate nodes needed to convert media types.
				hr = pSourceNode->ConnectOutput(0, pOutPutNode, 0);
			}

			//if (!previewOnly)
			//{
			//	// Create the sink node for the renderer.
			//	CComPtr<IMFTopologyNode> pFileWriterNode = NULL;
			//	if (SUCCEEDED_XOSb(hr))
			//	{
			//		hr = CreateFileWriterNode(pStreamDescriptor, pFileWriterNode);
			//	}
			//	if (SUCCEEDED_XOSb(hr) && pFileWriterNode)
			//	{
			//		// Add the sink node to the topology.
			//		hr = m_pMFTopology->AddNode(pFileWriterNode);
			//	}
			//	if (SUCCEEDED_XOSb(hr))
			//	{
			//		// Connect the source node to the sink node.  The resolver will find the
			//		// intermediate nodes needed to convert media types.
			//		hr = pSourceNode->ConnectOutput(0, pFileWriterNode, 0);
			//	}
			//}
		}
	}

	SUCCEEDED_XOSv(hr);
	return hr;
}

HRESULT XOSMFTopologyRep::CreateSourceStreamNode(CComPtr<IMFPresentationDescriptor> pPresDescriptor, CComPtr<IMFStreamDescriptor> pStreamDescriptor, CComPtr<IMFTopologyNode>& pNode, CComPtr<IMFMediaSource> pMFMediaSource)
{
	if (!pPresDescriptor || !pStreamDescriptor)
	{
		return E_INVALIDARG;
	}
	HRESULT hr = S_OK;
	if (SUCCEEDED_XOSb(hr))
	{
		// Create the topology node, indicating that it must be a source node.
		pNode = NULL;
		hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
	}

	if (SUCCEEDED_XOSb(hr) && pNode)
	{
		// Associate the node with the source by passing in a pointer to the media source
		// and indicating that it is the source
		hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pMFMediaSource);
	}

	if (SUCCEEDED_XOSb(hr) && pPresDescriptor)
	{
		// Set the node presentation descriptor attribute of the node by passing 
		// in a pointer to the presentation descriptor
		hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPresDescriptor);
	}

	if (SUCCEEDED_XOSb(hr) && pStreamDescriptor)
	{
		// Set the node stream descriptor attribute by passing in a pointer to the stream
		// descriptor
		hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pStreamDescriptor);
	}

	// if failed, clear the output parameter
	if (!SUCCEEDED_XOSb(hr))
	{
		pNode = NULL;
	}
		
	return hr;
}


HRESULT XOSMFTopologyRep::CreateFileWriterNode(CComPtr<IMFStreamDescriptor> pStreamDescriptor, CComPtr<IMFTopologyNode>& pNode)
{
	if (!pStreamDescriptor)
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

	CComPtr<IMFMediaTypeHandler> pHandler;
	if (!SUCCEEDED_XOSb(hr))
	{
		// Get the media type handler for the stream, which will be used to process
		// the media types of the stream.  The handler stores the media type.
		hr = pStreamDescriptor->GetMediaTypeHandler(&pHandler);
	}

	GUID majorType = GUID_NULL;
	if (!SUCCEEDED_XOSb(hr))
	{
		// Get the major media type (e.g. video or audio)
		hr = pHandler->GetMajorType(&majorType);
	}

	CComPtr<IMFActivate> pRendererActivate = NULL;
	if (!SUCCEEDED_XOSb(hr))
	{
		// Create an IMFActivate controller object for the renderer, based on the media type
		// The activation objects are used by the session in order to create the renderers 
		// only when they are needed - i.e. only right before starting playback.  The 
		// activation objects are also used to shut down the renderers.
		if (majorType == MFMediaType_Audio)
		{
			// if the stream major type is audio, create the audio renderer.
			hr = MFCreateAudioRendererActivate(&pRendererActivate);
		}
		else if (majorType == MFMediaType_Video)
		{
			// if the stream major type is video, create the video renderer, passing in the
			// video window handle - that's where the video will be playing.
			//hr = MFCreateVideoRendererActivate(hwndVideo, &pRendererActivate);
		}
		else
		{
			// fail if the stream type is not video or audio.
			// For example, fail if we encounter a CC stream.
			return E_FAIL;
		}
	}

	pNode = NULL;
	if (!SUCCEEDED_XOSb(hr))
	{
		// Create the node that will represent the renderer
		hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
	}

	if (!SUCCEEDED_XOSb(hr) && pNode)
	{
		// Store the IActivate object in the sink node - it will be extracted later by the
		// media session during the topology render phase.
		hr = pNode->SetObject(pRendererActivate);
	}

	// if failed, clear the output parameter
	if (!SUCCEEDED_XOSb(hr))
	{
		pNode = NULL;
	}

	return hr;
}

HRESULT XOSMFTopologyRep::CreateOutputNode(CComPtr<IMFStreamDescriptor> pStreamDescriptor, HWND hwndVideo, CComPtr<IMFTopologyNode>& pNode)
{
	HRESULT hr = S_OK;
	CComPtr<IMFMediaTypeHandler> pHandler = NULL;
	if (SUCCEEDED_XOSb(hr))
	{
		// Get the media type handler for the stream, which will be used to process
		// the media types of the stream.  The handler stores the media type.
		hr = pStreamDescriptor->GetMediaTypeHandler(&pHandler);
	}

	GUID majorType = GUID_NULL;
	if (SUCCEEDED_XOSb(hr) && pHandler)
	{
		// Get the major media type (e.g. video or audio)
		hr = pHandler->GetMajorType(&majorType);
	}

	CComPtr<IMFActivate> pRendererActivate = NULL;
	if (SUCCEEDED_XOSb(hr))
	{
		// Create an IMFActivate controller object for the renderer, based on the media type
		// The activation objects are used by the session in order to create the renderers 
		// only when they are needed - i.e. only right before starting playback.  The 
		// activation objects are also used to shut down the renderers.
		if (majorType == MFMediaType_Audio)
		{
			// if the stream major type is audio, create the audio renderer.
			hr = MFCreateAudioRendererActivate(&pRendererActivate);
		}
		else if (majorType == MFMediaType_Video)
		{
			// if the stream major type is video, create the video renderer, passing in the
			// video window handle - that's where the video will be playing.
			hr = MFCreateVideoRendererActivate(hwndVideo, &pRendererActivate);
		}
		else
		{
			// fail if the stream type is not video or audio.  For example, fail
			// if we encounter a CC stream.
			hr = E_FAIL;
		}
	}

	pNode = NULL;
	if (SUCCEEDED_XOSb(hr))
	{
		// Create the node that will represent the renderer
		hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
	}
	
	if (SUCCEEDED_XOSb(hr))
	{
		// Store the IActivate object in the sink node - it will be extracted later by the
		// media session during the topology render phase.
		hr = pNode->SetObject(pRendererActivate);
	}

	// if failed, clear the output parameter
	if (!SUCCEEDED_XOSb(hr))
	{
		pNode = NULL;
	}

	return hr;
}