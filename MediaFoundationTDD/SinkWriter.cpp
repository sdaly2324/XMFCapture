#include "SinkWriter.h"
#include "IMFWrapper.h"

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

private:
	CComPtr<IMFAttributes>	GetSinkWriterAttributesForCreation();
	CComPtr<IMFSinkWriter>	mSinkWriter = NULL;
};
SinkWriter::SinkWriter(LPCWSTR fullFilePath)
{
	m_pRep = std::unique_ptr<SinkWriterRep>(new SinkWriterRep(fullFilePath));
}
SinkWriterRep::SinkWriterRep(LPCWSTR fullFilePath)
{
	CComPtr<IMFAttributes> attributes = GetSinkWriterAttributesForCreation();
	if (attributes)
	{
		OnERR_return(MFCreateSinkWriterFromURL(fullFilePath, NULL, attributes, &mSinkWriter));
	}
}
CComPtr<IMFAttributes> SinkWriterRep::GetSinkWriterAttributesForCreation()
{
	CComPtr<IMFAttributes> attributes = NULL;
	OnERR_return_NULL(MFCreateAttributes(&attributes, 1));
	OnERR_return_NULL(attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	return attributes;
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