#include "stdafx.h"
#include "XMFCaptureEngine.h"

#include "XMFCaptureEngineWrapper.h"
#include "XMFUtilities.h"
#include "XMFCaptureDevice.h"
#include "XMFAVSourceReader.h"
#include "XMFSinkWriter.h"

#include <Mferror.h>
#include <Mfcaptureengine.h>

class XMFCaptureEngineRep
{
public:
	XMFCaptureEngineRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld);
	~XMFCaptureEngineRep();

	HRESULT StartPreview(HWND hwnd);
	HRESULT StopPreview();

	HRESULT StartRecord(PCWSTR pszDestinationFile);
	HRESULT StopRecord();

	bool IsPreviewing() const;
	bool IsRecording() const;
	HRESULT get_FramesCaptured(unsigned long* pVal) const;
	HRESULT	get_FPSForCapture(long* pVal) const;

private:
	XMFCaptureEngineRep();
	XMFCaptureEngineWrapper*				m_pCaptureEngineWrapper = NULL;
};
XMFCaptureEngine::XMFCaptureEngine(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld)
{
	m_pRep = new XMFCaptureEngineRep(pAudioDevice, pVideoDevice, useOld);
}
XMFCaptureEngineRep::XMFCaptureEngineRep(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld)
{
	m_pCaptureEngineWrapper = new XMFCaptureEngineWrapper(pAudioDevice, pVideoDevice, useOld);
}
XMFCaptureEngine::~XMFCaptureEngine()
{
	if (m_pRep)
	{
		delete m_pRep;
		m_pRep = NULL;
	}
}
XMFCaptureEngineRep::~XMFCaptureEngineRep()
{
}
HRESULT XMFCaptureEngine::StartRecord(PCWSTR pszDestinationFile)
{
	if (m_pRep)
	{
		return m_pRep->StartRecord(pszDestinationFile);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::StartRecord(PCWSTR pszDestinationFile)
{
	HRESULT hr = S_OK;
	PWSTR pszExt = PathFindExtension(pszDestinationFile);
	if (!(_wcsicmp(pszExt, L".mp4") == 0 || _wcsicmp(pszExt, L".ts") == 0))
	{
		return MF_E_INVALIDMEDIATYPE;
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureEngineWrapper->StartRecord(pszDestinationFile);
	}
	return hr;
}
HRESULT XMFCaptureEngine::StopRecord()
{
	if (m_pRep)
	{
		return m_pRep->StopRecord();
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::StopRecord()
{
	return m_pCaptureEngineWrapper->StopRecord();
}
bool XMFCaptureEngine::IsPreviewing() const
{
	if (m_pRep)
	{
		return m_pRep->IsPreviewing();
	}
	return false;
}
bool XMFCaptureEngineRep::IsPreviewing() const
{
	return m_pCaptureEngineWrapper->IsPreviewing();
}
bool XMFCaptureEngine::IsRecording() const
{
	if (m_pRep)
	{
		return m_pRep->IsRecording();
	}
	return false;
}
bool XMFCaptureEngineRep::IsRecording() const
{
	return m_pCaptureEngineWrapper->IsRecording();
}
HRESULT XMFCaptureEngine::get_FramesCaptured(unsigned long* pVal) const
{
	if (m_pRep)
	{
		return m_pRep->get_FramesCaptured(pVal);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::get_FramesCaptured(unsigned long* pVal) const
{
	if (m_pCaptureEngineWrapper)
	{
		return m_pCaptureEngineWrapper->get_FramesCaptured(pVal);
	}
	return E_FAIL;
}
HRESULT	XMFCaptureEngine::get_FPSForCapture(long* pVal) const
{
	if (m_pRep)
	{
		return m_pRep->get_FPSForCapture(pVal);
	}
	return E_FAIL;
}
HRESULT	XMFCaptureEngineRep::get_FPSForCapture(long* pVal) const
{
	if (m_pCaptureEngineWrapper)
	{
		return m_pCaptureEngineWrapper->get_FPSForCapture(pVal);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngine::StartPreview(HWND hwnd)
{
	if (m_pRep)
	{
		return m_pRep->StartPreview(hwnd);
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::StartPreview(HWND hwnd)
{
	if (m_pCaptureEngineWrapper == NULL)
	{
		return MF_E_NOT_INITIALIZED;
	}
	HRESULT hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureEngineWrapper->StartPreview(hwnd);
	}
	return hr;
}

HRESULT XMFCaptureEngine::StopPreview()
{
	if (m_pRep)
	{
		return m_pRep->StopPreview();
	}
	return E_FAIL;
}
HRESULT XMFCaptureEngineRep::StopPreview()
{
	if (m_pCaptureEngineWrapper == NULL)
	{
		return MF_E_NOT_INITIALIZED;
	}
	HRESULT hr = S_OK;
	if (SUCCEEDED_Xb(hr))
	{
		hr = m_pCaptureEngineWrapper->StopPreview();
	}
	return hr;
}