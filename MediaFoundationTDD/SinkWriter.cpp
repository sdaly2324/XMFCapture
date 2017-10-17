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
		PrintIfErrAndSave(MFCreateSinkWriterFromURL(fullFilePath, NULL, attributes, &mSinkWriter));
	}
}
CComPtr<IMFAttributes> SinkWriterRep::GetSinkWriterAttributesForCreation()
{
	CComPtr<IMFAttributes> attributes = NULL;
	PrintIfErrAndSave(MFCreateAttributes(&attributes, 1));
	if (LastHR_OK())
	{
		PrintIfErrAndSave(attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	}
	if (LastHR_OK())
	{
		return attributes;
	}
	return NULL;
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