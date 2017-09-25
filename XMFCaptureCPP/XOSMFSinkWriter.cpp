#include "XOSMFSinkWriter.h"
#include "XOSMFSinkWriterCallback.h"
#include "XOSMFSinkMediaTypes.h"
#include "XOSMFUtilities.h"

class XOSMFSinkWriterRep
{
public:
	XOSMFSinkWriterRep(LPCWSTR fullFilePath);
	~XOSMFSinkWriterRep();

	HRESULT ConfigureAudioEncoder(CComPtr<IMFMediaType> pAudioReaderInputMediaType, DWORD* pdwSinkStreamIndex);
	HRESULT ConfigureVideoEncoder(CComPtr<IMFMediaType> pVideoReaderInputMediaType, DWORD* pdwSinkStreamIndex);
	HRESULT WriteSample(DWORD dwStreamIndex, CComPtr<IMFSample> pSample, bool* bStopRequested);
	HRESULT AddStream(CComPtr<IMFMediaType> pTargetMediaType, DWORD* pdwStreamIndex);
	HRESULT SetInputMediaType(DWORD dwStreamIndex, CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFAttributes> pEncodingParameters);
	HRESULT BeginWriting();
	HRESULT EndWriting();
	HRESULT SignalAllStopped();

private:
	XOSMFSinkWriterRep();

	CComPtr<IMFSinkWriter> m_pSinkWriter;
	CComAutoCriticalSection m_protectSinkWriter;

	bool m_bStopRequested;
	HANDLE m_hEventAllStopped;
};

XOSMFSinkWriter::XOSMFSinkWriter(LPCWSTR fullFilePath)
{
	m_pRep = new XOSMFSinkWriterRep(fullFilePath);
}
XOSMFSinkWriterRep::XOSMFSinkWriterRep(LPCWSTR fullFilePath) :
m_pSinkWriter(NULL),
m_bStopRequested(false)
{
	m_hEventAllStopped = ::CreateEvent(NULL, false, false, L"TSFileWriter m_hEventAllStopped");

	// make sure we ask for hardware transforms (Intel quick sync)
	CComPtr<IMFAttributes> pAttributes = NULL;
	HRESULT hr = MFCreateAttributes(&pAttributes, 1);
	if (SUCCEEDED_XOSb(hr))
	{
		hr = pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		// for some reason SetUnknown does not count towards the IMFAttributes count
		hr = pAttributes->SetUnknown(MF_SINK_WRITER_ASYNC_CALLBACK, new XOSMFSinkWriterCallback());
	}

	if (SUCCEEDED_XOSb(hr))
	{
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		hr = MFCreateSinkWriterFromURL(fullFilePath, NULL, pAttributes, &m_pSinkWriter);
	}

	SUCCEEDED_XOSv(hr);
}

XOSMFSinkWriter::~XOSMFSinkWriter()
{
	if (m_pRep)
	{
		delete m_pRep;
	}
}
XOSMFSinkWriterRep::~XOSMFSinkWriterRep()
{
	CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
}

HRESULT XOSMFSinkWriter::WriteSample(DWORD dwStreamIndex, CComPtr<IMFSample> pSample, bool* bStopRequested)
{
	if (m_pRep)
	{
		return m_pRep->WriteSample(dwStreamIndex, pSample, bStopRequested);
	}
	return E_FAIL;
}
HRESULT XOSMFSinkWriterRep::WriteSample(DWORD dwStreamIndex, CComPtr<IMFSample> pSample, bool* bStopRequested)
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

HRESULT XOSMFSinkWriter::AddStream(CComPtr<IMFMediaType> pTargetMediaType, DWORD* pdwStreamIndex)
{
	if (m_pRep)
	{
		return m_pRep->AddStream(pTargetMediaType, pdwStreamIndex);
	}
	return E_FAIL;
}
HRESULT XOSMFSinkWriterRep::AddStream(CComPtr<IMFMediaType> pTargetMediaType, DWORD* pdwStreamIndex)
{
	if (m_pSinkWriter)
	{
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		return m_pSinkWriter->AddStream(pTargetMediaType, pdwStreamIndex);
	}
	return E_FAIL;
}

HRESULT XOSMFSinkWriter::SetInputMediaType(DWORD dwStreamIndex, CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFAttributes> pEncodingParameters)
{
	if (m_pRep)
	{
		return m_pRep->SetInputMediaType(dwStreamIndex, pInputMediaType, pEncodingParameters);
	}
	return E_FAIL;
}
HRESULT XOSMFSinkWriterRep::SetInputMediaType(DWORD dwStreamIndex, CComPtr<IMFMediaType> pInputMediaType, CComPtr<IMFAttributes> pEncodingParameters)
{
	if (m_pSinkWriter)
	{
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		return m_pSinkWriter->SetInputMediaType(dwStreamIndex, pInputMediaType, pEncodingParameters);
	}
	return E_FAIL;
}

HRESULT XOSMFSinkWriter::BeginWriting()
{
	if (m_pRep)
	{
		return m_pRep->BeginWriting();
	}
	return E_FAIL;
}
HRESULT XOSMFSinkWriterRep::BeginWriting()
{
	if (m_pSinkWriter)
	{
		CComCritSecLock<CComCriticalSection> lock(m_protectSinkWriter);
		return m_pSinkWriter->BeginWriting();
	}
	return E_FAIL;
}

HRESULT XOSMFSinkWriter::EndWriting()
{
	if (m_pRep)
	{
		return m_pRep->EndWriting();
	}
	return E_FAIL;
}
HRESULT XOSMFSinkWriterRep::EndWriting()
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

HRESULT XOSMFSinkWriter::SignalAllStopped()
{
	if (m_pRep)
	{
		return m_pRep->SignalAllStopped();
	}
	return E_FAIL;
}
HRESULT XOSMFSinkWriterRep::SignalAllStopped()
{
	SetEvent(m_hEventAllStopped);

	return S_OK;
}

HRESULT XOSMFSinkWriter::ConfigureAudioEncoder(CComPtr<IMFMediaType> pAudioReaderInputMediaType, DWORD* pdwSinkStreamIndex)
{
	if (m_pRep)
	{
		return m_pRep->ConfigureAudioEncoder(pAudioReaderInputMediaType, pdwSinkStreamIndex);
	}
	return E_FAIL;
}
HRESULT XOSMFSinkWriterRep::ConfigureAudioEncoder(CComPtr<IMFMediaType> pAudioReaderInputMediaType, DWORD* pdwSinkStreamIndex)
{
	HRESULT hr = S_OK;

	XOSMFAudioSinkMediaType aSinkMT;
	aSinkMT.ConfigFromSource(pAudioReaderInputMediaType);

	// Add the stream
	if (SUCCEEDED_XOSb(hr))
	{
		hr = AddStream(aSinkMT.GetMediaType(), pdwSinkStreamIndex);

		WCHAR mess[1024];
		swprintf_s(mess, 1024, L"stream(%d)", *pdwSinkStreamIndex);
		DumpAttr(aSinkMT.GetMediaType(), L"Audio Encoder", mess);
	}

	return hr;
}

HRESULT XOSMFSinkWriter::ConfigureVideoEncoder(CComPtr<IMFMediaType> pVideoReaderInputMediaType, DWORD* pdwSinkStreamIndex)
{
	if (m_pRep)
	{
		return m_pRep->ConfigureVideoEncoder(pVideoReaderInputMediaType, pdwSinkStreamIndex);
	}
	return E_FAIL;
}
HRESULT XOSMFSinkWriterRep::ConfigureVideoEncoder(CComPtr<IMFMediaType> pVideoReaderInputMediaType, DWORD* pdwSinkStreamIndex)
{
	HRESULT hr = S_OK;

	XOSMFVideoSinkMediaType vSinkMT;
	vSinkMT.ConfigFromSource(pVideoReaderInputMediaType);

	if (SUCCEEDED_XOSb(hr))
	{
		hr = AddStream(vSinkMT.GetMediaType(), pdwSinkStreamIndex);

		WCHAR mess[1024];
		swprintf_s(mess, 1024, L"stream(%d)", *pdwSinkStreamIndex);
		DumpAttr(vSinkMT.GetMediaType(), L"Video Encoder", mess);
	}

	return hr;
}