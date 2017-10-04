#include "XMFPreview.h"
#include "XMFCaptureDevice.h"
#include <evr.h>
#include <wmcodecdsp.h>
static XMFPreview* gInstance = NULL;
class XMFPreviewRep : public IMFAsyncCallback
{
public:
	XMFPreviewRep();
	virtual ~XMFPreviewRep();

	HRESULT StartPreview(HWND hWindowForVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceAudio);
	HRESULT StopPreview();
	void SetVideoDisplaySize(long width, long height);

	// IMFAsyncCallback
	STDMETHODIMP GetParameters(DWORD* /*pdwFlags*/, DWORD* /*pdwQueue*/)   { return E_NOTIMPL; }
	STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

private:
	HRESULT SetUpMediaSession(HWND hWindowForVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceAudio);
	HRESULT SetUpCaptureSourceNode(std::shared_ptr<XMFCaptureDevice> pXMFCaptureDevice, CComPtr<IMFTopologyNode>& pIMFTopologyNode);
	HRESULT SetUpEVRNode(HWND hWindowForVideo, CComPtr<IMFTopologyNode>& spNode);
	HRESULT SetUpSARNode(CComPtr<IMFTopologyNode>& pIMFTopologyNode);
	HRESULT ResolveTopology(CComPtr<IMFTopology> pIMFTopology, bool multipleSources, CComPtr<IMFTopology>& paIMFTopologyResolved);
	HRESULT SetTopology(CComPtr<IMFTopology> pIMFTopology, CComPtr<IMFMediaSession> pIMFMediaSession);
	HRESULT ProcessMediaEvent(CComPtr<IMFMediaEvent>& pMediaEvent);
	HRESULT OnTopologyReady();

	CComPtr<IMFMediaSession> m_pIMFMediaSession;
	CComPtr<IMFVideoDisplayControl> m_pVideoDisplay;
	CComAutoCriticalSection m_critSec;
	volatile long m_nRefCount;
};

XMFPreview::XMFPreview()
{
	m_pRep = new XMFPreviewRep();
}
XMFPreviewRep::XMFPreviewRep():
m_pIMFMediaSession(NULL),
m_pVideoDisplay(NULL),
m_nRefCount(1)
{
}

XMFPreview::~XMFPreview()
{
	if (m_pRep)
	{
		delete m_pRep;
		m_pRep = NULL;
	}
}
XMFPreviewRep::~XMFPreviewRep()
{
}

HRESULT	XMFPreview::StartPreview(HWND hWindowForVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceAudio)
{
	if (m_pRep)
	{
		return m_pRep->StartPreview(hWindowForVideo, pXMFCaptureDeviceVideo, pXMFCaptureDeviceAudio);
	}

	return E_FAIL;
}
HRESULT XMFPreviewRep::StartPreview(HWND hWindowForVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceAudio)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_Xb(hr))
	{
		hr = SetUpMediaSession(hWindowForVideo, pXMFCaptureDeviceVideo, pXMFCaptureDeviceAudio);
	}

	if (SUCCEEDED_Xb(hr))
	{
		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		varStart.vt = VT_EMPTY;
		hr = m_pIMFMediaSession->Start(&GUID_NULL, &varStart);
	}
	return hr;
}

HRESULT	XMFPreview::StopPreview()
{
	if (m_pRep)
	{
		return m_pRep->StopPreview();
	}

	return E_FAIL;
}
HRESULT XMFPreviewRep::StopPreview()
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pIMFMediaSession->Stop();
	}
	return hr;
}

void XMFPreview::SetVideoDisplaySize(long width, long height)
{
	if (m_pRep)
	{
		m_pRep->SetVideoDisplaySize(width, height);
	}
}
void XMFPreviewRep::SetVideoDisplaySize(long width, long height)
{
	if (m_pVideoDisplay)
	{
		RECT displayRect = { 0, 0, width, height };
		HRESULT hr = m_pVideoDisplay->SetVideoPosition(NULL, &displayRect);
		SUCCEEDED_Xv(hr);
	}
}

HRESULT XMFPreviewRep::SetUpCaptureSourceNode(std::shared_ptr<XMFCaptureDevice> pXMFCaptureDevice, CComPtr<IMFTopologyNode>& paIMFTopologyNode)
{
	HRESULT hr = S_OK;

	CComPtr<IMFMediaSource> pIMFMediaSource = NULL;
	if (SUCCEEDED_Xb(hr) && pXMFCaptureDevice)
	{
		hr = pXMFCaptureDevice->GetIMFMediaSource(pIMFMediaSource);
	}
	CComPtr<IMFPresentationDescriptor> pIMFPresentationDescriptor = NULL;
	if (SUCCEEDED_Xb(hr) && pIMFMediaSource)
	{
		hr = pIMFMediaSource->CreatePresentationDescriptor(&pIMFPresentationDescriptor);
	}
	DWORD cSourceStreams = 0;
	if (SUCCEEDED_Xb(hr) && pIMFPresentationDescriptor)
	{
		hr = pIMFPresentationDescriptor->GetStreamDescriptorCount(&cSourceStreams);
	}
	if (SUCCEEDED_Xb(hr))
	{
		DWORD streamIndex = 0;
		for (streamIndex = 0; streamIndex < cSourceStreams; streamIndex++)
		{
			CComPtr<IMFStreamDescriptor> pIMFStreamDescriptor = NULL;
			BOOL isSelected = FALSE;
			if (SUCCEEDED_Xb(hr))
			{
				hr = pIMFPresentationDescriptor->GetStreamDescriptorByIndex(streamIndex, &isSelected, &pIMFStreamDescriptor);
			}
			if (SUCCEEDED_Xb(hr))
			{
				hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &paIMFTopologyNode);
			}
			if (SUCCEEDED_Xb(hr) && paIMFTopologyNode)
			{
				hr = paIMFTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, pIMFMediaSource);
			}
			if (SUCCEEDED_Xb(hr) && paIMFTopologyNode)
			{
				hr = paIMFTopologyNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pIMFPresentationDescriptor);
			}
			if (SUCCEEDED_Xb(hr) && paIMFTopologyNode && pIMFStreamDescriptor)
			{
				hr = paIMFTopologyNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pIMFStreamDescriptor);
			}
		}
	}
	return hr;
}

HRESULT XMFPreviewRep::SetUpEVRNode(HWND hWindowForVideo, CComPtr<IMFTopologyNode>& paIMFTopologyNode)
{
	HRESULT hr = S_OK;

	CComPtr<IMFActivate> pIMFActivate = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateVideoRendererActivate(hWindowForVideo, &pIMFActivate);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &paIMFTopologyNode);
	}
	if (SUCCEEDED_Xb(hr) && pIMFActivate && paIMFTopologyNode)
	{
		hr = paIMFTopologyNode->SetObject(pIMFActivate);
	}
	return hr;
}

HRESULT XMFPreviewRep::SetUpSARNode(CComPtr<IMFTopologyNode>& pIMFTopologyNode)
{
	HRESULT hr = S_OK;

	CComPtr<IMFActivate> pIMFActivate = NULL;
	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateAudioRendererActivate(&pIMFActivate);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pIMFTopologyNode);
	}
	if (SUCCEEDED_Xb(hr) && pIMFActivate && pIMFTopologyNode)
	{
		hr = pIMFTopologyNode->SetObject(pIMFActivate);
	}
	return hr;
}

HRESULT XMFPreviewRep::ResolveTopology(CComPtr<IMFTopology> pIMFTopology, bool multipleSources, CComPtr<IMFTopology>& paIMFTopologyResolved)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_Xb(hr))
	{
		hr = MFCreateTopology(&paIMFTopologyResolved);
	}
	if (SUCCEEDED_Xb(hr) && paIMFTopologyResolved)
	{
		hr = paIMFTopologyResolved->CloneFrom(pIMFTopology);
	}

	WORD nodeCount = 0;
	if (SUCCEEDED_Xb(hr))
	{
		hr = paIMFTopologyResolved->GetNodeCount(&nodeCount);
	}
	if (SUCCEEDED_Xb(hr))
	{
		for (WORD i = 0; i < nodeCount; i++)
		{
			CComPtr<IMFTopologyNode> pIMFTopologyNode = NULL;
			hr = paIMFTopologyResolved->GetNode(i, &pIMFTopologyNode);
			if (SUCCEEDED_Xb(hr) && pIMFTopologyNode)
			{
				hr = pIMFTopologyNode->DeleteItem(MF_TOPONODE_ERRORCODE);
			}
			if (SUCCEEDED_Xb(hr) && pIMFTopologyNode)
			{
				hr = pIMFTopologyNode->DeleteItem(MF_TOPONODE_ERROR_MAJORTYPE);
			}
			if (SUCCEEDED_Xb(hr) && pIMFTopologyNode)
			{
				hr = pIMFTopologyNode->DeleteItem(MF_TOPONODE_ERROR_SUBTYPE);
			}
			
			bool isConnected = false;
			DWORD inputCount = 0;
			if (SUCCEEDED_Xb(hr) && pIMFTopologyNode)
			{
				hr = pIMFTopologyNode->GetInputCount(&inputCount);
			}
			if (SUCCEEDED_Xb(hr) && pIMFTopologyNode)
			{
				for (DWORD j = 0; j < inputCount; j++)
				{
					CComPtr<IMFTopologyNode> pIMFTopologyNodeUp;
					DWORD upIndex;
					hr = pIMFTopologyNode->GetInput(j, &pIMFTopologyNodeUp, &upIndex);
					if (SUCCEEDED_Xb(hr))
					{
						isConnected = true;
						break;
					}
				}
			}
			if (!isConnected)
			{
				DWORD outputCount = 0;
				if (SUCCEEDED_Xb(hr) && pIMFTopologyNode)
				{
					hr = pIMFTopologyNode->GetOutputCount(&outputCount);
				}
				if (SUCCEEDED_Xb(hr) && pIMFTopologyNode)
				{
					for (DWORD j = 0; j < outputCount; j++)
					{
						CComPtr<IMFTopologyNode> pIMFTopologyNodeDown;
						DWORD downIndex;
						hr = pIMFTopologyNode->GetOutput(j, &pIMFTopologyNodeDown, &downIndex);
						if (SUCCEEDED_Xb(hr))
						{
							isConnected = true;
							break;
						}
					}
				}
			}
			if (!isConnected)
			{
				hr = paIMFTopologyResolved->RemoveNode(pIMFTopologyNode);
				if (SUCCEEDED_Xb(hr))
				{
					nodeCount--;
					i--;
				}
			}
		}
	}
	
	if (multipleSources)
	{
		CComPtr<IMFSequencerSource> pIMFSequencerSource = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			hr = MFCreateSequencerSource(NULL, &pIMFSequencerSource);
		}
		MFSequencerElementId newMFSequencerElementId = 0;
		if (SUCCEEDED_Xb(hr))
		{
			hr = pIMFSequencerSource->AppendTopology(paIMFTopologyResolved, SequencerTopologyFlags_Last, &newMFSequencerElementId);
		}

		CComPtr<IMFMediaSource> pIMFMediaSource = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			hr = pIMFSequencerSource->QueryInterface(IID_IMFMediaSource, (void**)&pIMFMediaSource);
		}

		CComPtr<IMFPresentationDescriptor> pIMFPresentationDescriptor = NULL;
		if (SUCCEEDED_Xb(hr) && pIMFMediaSource)
		{
			hr = pIMFMediaSource->CreatePresentationDescriptor(&pIMFPresentationDescriptor);
		}

		CComPtr<IMFMediaSourceTopologyProvider> pIMFMediaSourceTopologyProvider = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			hr = pIMFSequencerSource->QueryInterface(IID_IMFMediaSourceTopologyProvider, (void**)&pIMFMediaSourceTopologyProvider);
		}

		paIMFTopologyResolved.Release();
		if (SUCCEEDED_Xb(hr) && pIMFMediaSourceTopologyProvider && pIMFPresentationDescriptor)
		{
			hr = pIMFMediaSourceTopologyProvider->GetMediaSourceTopology(pIMFPresentationDescriptor, &paIMFTopologyResolved);
		}
	}
	return hr;
}

HRESULT XMFPreviewRep::SetTopology(CComPtr<IMFTopology> pIMFTopology, CComPtr<IMFMediaSession> pIMFMediaSession)
{
	HRESULT hr = S_OK;
	
	// Set topology attributes to enable new MF features.  HWMODE_USE_HARDWARE allows us
	// to pick up hardware MFTs for decoding.  DXVA_FULL allows us to enable full
	// DXVA resolution for the topology
	if (SUCCEEDED_Xb(hr))
	{
		hr = pIMFTopology->SetUINT32(MF_TOPOLOGY_HARDWARE_MODE, MFTOPOLOGY_HWMODE_USE_HARDWARE);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pIMFTopology->SetUINT32(MF_TOPOLOGY_DXVA_MODE, MFTOPOLOGY_DXVA_FULL);
	}

	if (SUCCEEDED_Xb(hr))
	{
		hr = pIMFMediaSession->SetTopology(0, pIMFTopology);
	}
	return hr;
}

HRESULT XMFPreviewRep::SetUpMediaSession(HWND hWindowForVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceVideo, std::shared_ptr<XMFCaptureDevice> pXMFCaptureDeviceAudio)
{
	HRESULT hr = S_OK;

	// The Topology
	if (m_pIMFMediaSession == NULL)
	{
		CComPtr<IMFTopology> pIMFTopology = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			hr = MFCreateTopology(&pIMFTopology);
		}
		if (!SUCCEEDED_Xb(hr))
		{
			return hr;
		}
		if (pIMFTopology == NULL)
		{
			return E_FAIL;
		}

		// VIDEO
		if (pXMFCaptureDeviceVideo)
		{
			// Capture Source
			CComPtr<IMFTopologyNode> pIMFTopologyNodeVideo = NULL;
			if (SUCCEEDED_Xb(hr))
			{
				hr = SetUpCaptureSourceNode(pXMFCaptureDeviceVideo, pIMFTopologyNodeVideo);
			}
			if (SUCCEEDED_Xb(hr) && pIMFTopologyNodeVideo)
			{
				hr = pIMFTopology->AddNode(pIMFTopologyNodeVideo);
			}

			// Video Renderer
			CComPtr<IMFTopologyNode> pIMFTopologyNodeEVR = NULL;
			if (SUCCEEDED_Xb(hr))
			{
				hr = SetUpEVRNode(hWindowForVideo, pIMFTopologyNodeEVR);
			}
			if (SUCCEEDED_Xb(hr))
			{
				hr = pIMFTopology->AddNode(pIMFTopologyNodeEVR);
			}

			// Connect
			if (SUCCEEDED_Xb(hr) && pIMFTopologyNodeVideo && pIMFTopologyNodeEVR)
			{
				hr = pIMFTopologyNodeVideo->ConnectOutput(0, pIMFTopologyNodeEVR, 0);
			}
		}

		// AUDIO
		if (pXMFCaptureDeviceAudio)
		{
			// Capture Source
			CComPtr<IMFTopologyNode> pIMFTopologyNodeAudio = NULL;
			if (SUCCEEDED_Xb(hr))
			{
				hr = SetUpCaptureSourceNode(pXMFCaptureDeviceAudio, pIMFTopologyNodeAudio);
			}
			if (SUCCEEDED_Xb(hr))
			{
				hr = pIMFTopology->AddNode(pIMFTopologyNodeAudio);
			}

			// Audio Renderer
			CComPtr<IMFTopologyNode> pIMFTopologyNodeSAR = NULL;
			if (SUCCEEDED_Xb(hr))
			{
				hr = SetUpSARNode(pIMFTopologyNodeSAR);
			}
			if (SUCCEEDED_Xb(hr))
			{
				hr = pIMFTopology->AddNode(pIMFTopologyNodeSAR);
			}

			// Connect
			if (SUCCEEDED_Xb(hr) && pIMFTopologyNodeAudio && pIMFTopologyNodeSAR)
			{
				hr = pIMFTopologyNodeAudio->ConnectOutput(0, pIMFTopologyNodeSAR, 0);
			}
		}

		// Resolve Topology
		CComPtr<IMFTopology> pIMFTopologyResolved = NULL;
		if (SUCCEEDED_Xb(hr))
		{
			hr = ResolveTopology(pIMFTopology, pXMFCaptureDeviceVideo && pXMFCaptureDeviceAudio, pIMFTopologyResolved);
		}
		pIMFTopology.Release();

		if (SUCCEEDED_Xb(hr))
		{
			hr = MFCreateMediaSession(NULL, &m_pIMFMediaSession);
		}
		if (SUCCEEDED_Xb(hr) && pIMFTopologyResolved)
		{
			hr = SetTopology(pIMFTopologyResolved, m_pIMFMediaSession);
		}
		if (SUCCEEDED_Xb(hr))
		{
			hr = m_pIMFMediaSession->BeginGetEvent((IMFAsyncCallback*)this, NULL);
		}
	}
	return hr;
}

HRESULT XMFPreviewRep::Invoke(IMFAsyncResult* pAsyncResult)
{
	CComPtr<IMFMediaEvent> pEvent;
	HRESULT hr = S_OK;

	do
	{
		CComCritSecLock<CComAutoCriticalSection> lock(m_critSec);

		BREAK_ON_NULL(pAsyncResult, E_UNEXPECTED);

		// Get the event from the event queue.
		hr = m_pIMFMediaSession->EndGetEvent(pAsyncResult, &pEvent);
		BREAK_ON_FAIL(hr);

		hr = ProcessMediaEvent(pEvent);
		BREAK_ON_FAIL(hr);

		if (hr != S_FALSE)
		{
			hr = m_pIMFMediaSession->BeginGetEvent(this, NULL);
			BREAK_ON_FAIL(hr);
		}
	} while (false);

	return S_OK;
}

HRESULT XMFPreviewRep::ProcessMediaEvent(CComPtr<IMFMediaEvent>& pMediaEvent)
{
	HRESULT hrStatus = S_OK;            // Event status
	HRESULT hr = S_OK;
	UINT32 TopoStatus = MF_TOPOSTATUS_INVALID;
	MediaEventType eventType;

	do
	{
		BREAK_ON_NULL(pMediaEvent, E_POINTER);
		// Get the event type.
		hr = pMediaEvent->GetType(&eventType);
		BREAK_ON_FAIL(hr);
		// Get the event status. If the operation that triggered the event did
		// not succeed, the status is a failure code.
		hr = pMediaEvent->GetStatus(&hrStatus);
		BREAK_ON_FAIL(hr);
		// Check if the async operation succeeded.
		if (FAILED(hrStatus))
		{
			WCHAR mess[256];
			swprintf(mess, 256, L"XMFPreviewRep::ProcessMediaEvent ERROR(%d) (0x%x)!!!!!!!!!!!!!!\n", hrStatus, hrStatus);
			OutputDebugString(mess);

			hr = hrStatus;
			break;
		}
		if (eventType == MESessionTopologyStatus)
		{
			hr = pMediaEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus);
			BREAK_ON_FAIL(hr);

			if (TopoStatus == MF_TOPOSTATUS_READY)
			{
				hr = OnTopologyReady();
			}
		}
	} while (false);

	return hr;
}

HRESULT XMFPreviewRep::OnTopologyReady()
{
	m_pVideoDisplay.Release();
	HRESULT hr = MFGetService(m_pIMFMediaSession, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&m_pVideoDisplay);
	SUCCEEDED_Xv(hr);
	return hr;
}

HRESULT XMFPreviewRep::QueryInterface(REFIID riid, void** ppv)
{
	HRESULT hr = S_OK;

	if (ppv == NULL)
		return E_POINTER;

	if (riid == __uuidof(IMFAsyncCallback))
		*ppv = static_cast<IMFAsyncCallback*>(this);
	else if (riid == __uuidof(IUnknown))
		*ppv = static_cast<IUnknown*>(this);
	else
	{
		*ppv = NULL;
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr))
		AddRef();

	return hr;
}

ULONG XMFPreviewRep::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}

ULONG XMFPreviewRep::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}