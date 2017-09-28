#include "stdafx.h"
#include "XMFSinkWriter.h"
#include "XMFSinkWriterCallback.h"
#include "XMFUtilities.h"

class XMFSinkWriterRep
{
public:
	XMFSinkWriterRep(LPCWSTR fullFilePath);
	~XMFSinkWriterRep();

	HRESULT WriteSample(DWORD dwStreamIndex, CComPtr<IMFSample> pSample, bool* bStopRequested);
	HRESULT AddStream(CComPtr<IMFMediaType> pTargetMediaType, DWORD* pdwStreamIndex);
	HRESULT SetInputMediaType(DWORD dwStreamIndex, CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFAttributes> pEncodingParameters);
	HRESULT BeginWriting();
	HRESULT EndWriting();
	HRESULT SignalAllStopped();
	HRESULT GetStatistics(DWORD dwStreamIndex, MF_SINK_WRITER_STATISTICS *pStats);

private:
	XMFSinkWriterRep();

	CComPtr<IMFSinkWriter> m_pSinkWriter;
	CComAutoCriticalSection m_protectSinkWriter;

	bool m_bStopRequested;
	HANDLE m_hEventAllStopped;
};

XMFSinkWriter::XMFSinkWriter(LPCWSTR fullFilePath)
{
	m_pRep = new XMFSinkWriterRep(fullFilePath);
}
XMFSinkWriterRep::XMFSinkWriterRep(LPCWSTR fullFilePath) :
m_pSinkWriter(NULL),
m_bStopRequested(false)
{
	m_hEventAllStopped = ::CreateEvent(NULL, false, false, L"TSFileWriter m_hEventAllStopped");

	// make sure we ask for hardware transforms (Intel quick sync)
	CComPtr<IMFAttributes> pAttributes = NULL;
	HRESULT hr = MFCreateAttributes(&pAttributes, 1);
	if (SUCCEEDED_Xb(hr))
	{
		hr = pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
	}

	if (SUCCEEDED_Xb(hr))
	{
		// for some reason SetUnknown does not count towards the IMFAttributes count
		hr = pAttributes->SetUnknown(MF_SINK_WRITER_ASYNC_CALLBACK, new XMFSinkWriterCallback());
	}

	if (SUCCEEDED_Xb(hr))
	{
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		hr = MFCreateSinkWriterFromURL(fullFilePath, NULL, pAttributes, &m_pSinkWriter);
	}

	SUCCEEDED_Xv(hr);
}

XMFSinkWriter::~XMFSinkWriter()
{
	if (m_pRep)
	{
		delete m_pRep;
	}
}
XMFSinkWriterRep::~XMFSinkWriterRep()
{
	CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
}

HRESULT XMFSinkWriter::WriteSample(DWORD dwStreamIndex, CComPtr<IMFSample> pSample, bool* bStopRequested)
{
	if (m_pRep)
	{
		return m_pRep->WriteSample(dwStreamIndex, pSample, bStopRequested);
	}
	return E_FAIL;
}
HRESULT XMFSinkWriterRep::WriteSample(DWORD dwStreamIndex, CComPtr<IMFSample> pSample, bool* bStopRequested)
{
	if (m_pSinkWriter)
	{
		if (bStopRequested != NULL)
		{
			*bStopRequested = m_bStopRequested;
		}
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		if (m_pSinkWriter)
		{
			return m_pSinkWriter->WriteSample(dwStreamIndex, pSample);
		}
	}
	return E_FAIL;
}

HRESULT XMFSinkWriter::AddStream(CComPtr<IMFMediaType> pTargetMediaType, DWORD* pdwStreamIndex)
{
	if (m_pRep)
	{
		return m_pRep->AddStream(pTargetMediaType, pdwStreamIndex);
	}
	return E_FAIL;
}
HRESULT XMFSinkWriterRep::AddStream(CComPtr<IMFMediaType> pTargetMediaType, DWORD* pdwStreamIndex)
{
	if (m_pSinkWriter)
	{
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		return m_pSinkWriter->AddStream(pTargetMediaType, pdwStreamIndex);
	}
	return E_FAIL;
}

HRESULT XMFSinkWriter::SetInputMediaType(DWORD dwStreamIndex, CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFAttributes> pEncodingParameters)
{
	if (m_pRep)
	{
		return m_pRep->SetInputMediaType(dwStreamIndex, pInputMediaType, pEncodingParameters);
	}
	return E_FAIL;
}
HRESULT XMFSinkWriterRep::SetInputMediaType(DWORD dwStreamIndex, CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFAttributes> pEncodingParameters)
{
	if (m_pSinkWriter)
	{
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		return m_pSinkWriter->SetInputMediaType(dwStreamIndex, pInputMediaType, pEncodingParameters);
	}
	return E_FAIL;
}

HRESULT XMFSinkWriter::BeginWriting()
{
	if (m_pRep)
	{
		return m_pRep->BeginWriting();
	}
	return E_FAIL;
}
HRESULT XMFSinkWriterRep::BeginWriting()
{
	if (m_pSinkWriter)
	{
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		return m_pSinkWriter->BeginWriting();
	}
	return E_FAIL;
}

HRESULT XMFSinkWriter::EndWriting()
{
	if (m_pRep)
	{
		return m_pRep->EndWriting();
	}
	return E_FAIL;
}
HRESULT XMFSinkWriterRep::EndWriting()
{
	if (m_pSinkWriter)
	{
		m_bStopRequested = true;

		WaitForSingleObject(m_hEventAllStopped, INFINITE);

		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);

		OutputDebugStringA("m_pSinkWriter->Finalize START\n");
		HRESULT hr = m_pSinkWriter->Finalize();
		OutputDebugStringA("m_pSinkWriter->Finalize END\n");
		return hr;
	}
	return E_FAIL;
}

HRESULT XMFSinkWriter::SignalAllStopped()
{
	if (m_pRep)
	{
		return m_pRep->SignalAllStopped();
	}
	return E_FAIL;
}
HRESULT XMFSinkWriterRep::SignalAllStopped()
{
	SetEvent(m_hEventAllStopped);
	return S_OK;
}

HRESULT XMFSinkWriter::GetStatistics(DWORD dwStreamIndex, MF_SINK_WRITER_STATISTICS *pStats)
{
	if (m_pRep)
	{
		return m_pRep->GetStatistics(dwStreamIndex, pStats);
	}
	return E_FAIL;
}
HRESULT XMFSinkWriterRep::GetStatistics(DWORD dwStreamIndex, MF_SINK_WRITER_STATISTICS *pStats)
{
	if (m_pSinkWriter)
	{
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		if (m_pSinkWriter)
		{
			return m_pSinkWriter->GetStatistics(dwStreamIndex, pStats);
		}
	}
	return E_FAIL;
}