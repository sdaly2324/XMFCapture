#include "SinkWriter.h"
#include "IMFWrapper.h"
#include "AttributesFactory.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class SinkWriterRep : public IMFWrapper
{
public:
	SinkWriterRep(LPCWSTR fullFilePath);
	~SinkWriterRep();

	HRESULT					GetLastHRESULT();

	CComPtr<IMFSinkWriter>	GetSinkWriter();
	CComPtr<IMFMediaSink>	GetMediaSink();

private:
	CComPtr<IMFSinkWriter>	mSinkWriter = NULL;
};
SinkWriter::SinkWriter(LPCWSTR fullFilePath)
{
	m_pRep = std::unique_ptr<SinkWriterRep>(new SinkWriterRep(fullFilePath));
}
SinkWriterRep::SinkWriterRep(LPCWSTR fullFilePath)
{
	AttributesFactory attributesFactory;
	CComPtr<IMFAttributes> attributes = attributesFactory.CreateSinkWriterAttributes();
	if (attributes)
	{
		OnERR_return(MFCreateSinkWriterFromURL(fullFilePath, NULL, attributes, &mSinkWriter));
	}
}
SinkWriter::~SinkWriter()
{
}
SinkWriterRep::~SinkWriterRep()
{
}

HRESULT SinkWriter::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT SinkWriterRep::GetLastHRESULT()
{
	return IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFSinkWriter> SinkWriter::GetSinkWriter()
{
	return m_pRep->GetSinkWriter();
}
CComPtr<IMFSinkWriter> SinkWriterRep::GetSinkWriter()
{
	return mSinkWriter;
}

CComPtr<IMFMediaSink> SinkWriter::GetMediaSink()
{
	return m_pRep->GetMediaSink();
}
CComPtr<IMFMediaSink> SinkWriterRep::GetMediaSink()
{
	CComPtr<IMFMediaSink> mediaSink = NULL;
	OnERR_return_NULL(mSinkWriter->GetServiceForStream(MF_SINK_WRITER_MEDIASINK, GUID_NULL, IID_IMFMediaSink, (LPVOID*)&mediaSink));
	return mediaSink;
}