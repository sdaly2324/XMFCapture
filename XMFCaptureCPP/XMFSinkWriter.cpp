#include "XMFSinkWriter.h"
#include "XMFSinkWriterCallback.h"
#include "XMFUtilities.h"

#include <strmif.h>
#include <codecapi.h>

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

	CComPtr<IMFSinkWriter> m_pSinkWriter = NULL;
	CComAutoCriticalSection m_protectSinkWriter;

	bool m_bStopRequested = false;
	HANDLE m_hEventAllStopped = INVALID_HANDLE_VALUE;
	UINT32 m_GOPSize = 0;
	DWORD m_VideoStreamIndex = 0;
};

XMFSinkWriter::XMFSinkWriter(LPCWSTR fullFilePath)
{
	m_pRep = new XMFSinkWriterRep(fullFilePath);
}
XMFSinkWriterRep::XMFSinkWriterRep(LPCWSTR fullFilePath)
{
	m_hEventAllStopped = ::CreateEvent(NULL, false, false, L"TSFileWriter m_hEventAllStopped");

	// make sure we ask for hardware transforms (Intel quick sync)
	CComPtr<IMFAttributes> pAttributes = NULL;
	HRESULT hr = MFCreateAttributes(&pAttributes, 0);
	if (SUCCEEDED_Xb(hr))
	{
		hr = pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pAttributes->SetUnknown(MF_SINK_WRITER_ASYNC_CALLBACK, new XMFSinkWriterCallback());
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pAttributes->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Stream);
	}
	if (SUCCEEDED_Xb(hr))
	{
		hr = pAttributes->SetGUID(MF_MT_SUBTYPE, MFStreamFormat_MPEG2Transport);
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
		HRESULT hr = m_pSinkWriter->AddStream(pTargetMediaType, pdwStreamIndex);
		if (SUCCEEDED_Xb(hr))
		{
			UINT32 GOPSize = 0;
			HRESULT temp = pTargetMediaType->GetUINT32(MF_MT_MAX_KEYFRAME_SPACING, &GOPSize);
			if (SUCCEEDED_Xb(temp))
			{
				m_GOPSize = GOPSize;
				m_VideoStreamIndex = *pdwStreamIndex;
			}
		}
		return hr;
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
		HRESULT hr = S_OK;
		if (m_GOPSize > 0)
		{
			// HACK BECAUSE setting the IMFMediaType with this attribute MF_MT_MAX_KEYFRAME_SPACING set to 30
			// does not work when we call IMFSinkWriter::AddAddStream, it has to happen later
			CComPtr<IMFTransform> videoDecoder = NULL;
			if (SUCCEEDED_Xb(hr))
			{
				hr = m_pSinkWriter->GetServiceForStream(m_VideoStreamIndex, GUID_NULL, IID_IMFTransform, (LPVOID*)&videoDecoder);
			}
			CComPtr<ICodecAPI> codecApi;
			if (SUCCEEDED_Xb(hr))
			{
				hr = videoDecoder->QueryInterface(&codecApi);
			}
			VARIANT GOPSize;
			VariantInit(&GOPSize);
			if (SUCCEEDED_Xb(hr))
			{
				hr = codecApi->GetValue(&CODECAPI_AVEncMPVGOPSize, &GOPSize);
			}
			if (SUCCEEDED_Xb(hr))
			{
				GOPSize.intVal = m_GOPSize;
				hr = codecApi->SetValue(&CODECAPI_AVEncMPVGOPSize, &GOPSize);
			}
			VariantClear(&GOPSize);
		}
		if (SUCCEEDED_Xb(hr))
		{
			hr = m_pSinkWriter->BeginWriting();
		}
		return hr;
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