#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFSinkWriter;
struct IMFMediaSink;
class SinkWriterRep;
class MediaFoundationTDD_API SinkWriter
{
public:
	SinkWriter(LPCWSTR fullFilePath);
	~SinkWriter();

	HRESULT							GetLastHRESULT();

	CComPtr<IMFSinkWriter>			GetSinkWriter();
	CComPtr<IMFMediaSink>			GetMediaSink();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<SinkWriterRep> m_pRep = 0;
#pragma warning(pop)
};
