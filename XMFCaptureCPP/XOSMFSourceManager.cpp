#include "XOSMFSourceManager.h"
#include "XOSMFAVSourceReader.h"

XOSMFSourceManager::XOSMFSourceManager(XOSMFCaptureDevice* pAudioDevice, XOSMFCaptureDevice* pVideoDevice, XOSMFSinkWriter* pXOSMFSinkWriter) :
	m_pXOSMFAVSourceReader(NULL)
{
	HRESULT hr = S_OK;

	CComPtr<IMFMediaSource>	pVASource = NULL;

	CComPtr<IMFMediaSource> pVideoSource = NULL;
	if (SUCCEEDED_XOSb(hr) && pVideoDevice)
	{
		hr = pVideoDevice->ActivateDevice(pVideoSource);
		pVASource = pVideoSource;
	}

	CComPtr<IMFMediaSource> pAudioSource = NULL;
	if (SUCCEEDED_XOSb(hr) && pAudioDevice)
	{
		hr = pAudioDevice->ActivateDevice(pAudioSource);
		pVASource = pAudioSource;
	}

	if (SUCCEEDED_XOSb(hr) && pAudioSource && pVideoSource)
	{
		hr = CreateAggregateMediaSource(pVideoSource, pAudioSource, pVASource);
	}

	m_pXOSMFAVSourceReader = new XOSMFAVSourceReader(pXOSMFSinkWriter, pVASource, pAudioDevice ? true : false, pVideoDevice ? true : false);

	SUCCEEDED_XOSv(hr);
}

XOSMFSourceManager::~XOSMFSourceManager()
{
	if (m_pXOSMFAVSourceReader)
	{
		delete m_pXOSMFAVSourceReader;
		m_pXOSMFAVSourceReader = NULL;
	}
}

HRESULT XOSMFSourceManager::CreateAggregateMediaSource(CComPtr<IMFMediaSource> pMFVMediaSource, CComPtr<IMFMediaSource> pMFAMediaSource, CComPtr<IMFMediaSource>& pMFAVMediaSource)
{
	CComPtr<IMFCollection> pCollection = NULL;

	HRESULT hr = MFCreateCollection(&pCollection);

	if (SUCCEEDED_XOSb(hr))
	{
		hr = pCollection->AddElement(pMFVMediaSource);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = pCollection->AddElement(pMFAMediaSource);
	}

	IMFMediaSource* temp = NULL;
	if (SUCCEEDED_XOSb(hr))
	{
		hr = MFCreateAggregateSource(pCollection, &temp);
	}
	if (SUCCEEDED_XOSb(hr) && temp)
	{
		pMFAVMediaSource = temp;
		temp->Release();
	}

	return hr;
}

HRESULT XOSMFSourceManager::Start()
{
	if (m_pXOSMFAVSourceReader)
	{
		return m_pXOSMFAVSourceReader->Start();
	}
	return E_UNEXPECTED;
}

HRESULT XOSMFSourceManager::GetVideoInputMediaType(CComPtr<IMFMediaType>& pVideoInputMediaType)
{
	return m_pXOSMFAVSourceReader->GetVideoInputMediaType(pVideoInputMediaType);
}

HRESULT XOSMFSourceManager::GetAudioInputMediaType(CComPtr<IMFMediaType>& pAudioInputMediaType)
{
	return m_pXOSMFAVSourceReader->GetAudioInputMediaType(pAudioInputMediaType);
}

bool XOSMFSourceManager::Capturing()
{
	if (m_pXOSMFAVSourceReader)
	{
		return m_pXOSMFAVSourceReader->Capturing();
	}

	return false;
}

HRESULT XOSMFSourceManager::GetXOSMFAVSourceReader(XOSMFAVSourceReader** ppXOSMFAVSourceReader)
{
	if (ppXOSMFAVSourceReader == NULL)
	{
		return E_INVALIDARG;
	}
	
	*ppXOSMFAVSourceReader = NULL;
	if (m_pXOSMFAVSourceReader)
	{
		*ppXOSMFAVSourceReader = m_pXOSMFAVSourceReader;
		return S_OK;
	}

	return E_FAIL;
}