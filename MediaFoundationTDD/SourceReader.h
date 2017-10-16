#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSource;
struct IMFSourceReader;
struct IMFPresentationDescriptor;
struct IMFStreamDescriptor;
class SourceReaderRep;
class MediaFoundationTDD_API SourceReader
{
public:
	SourceReader(CComPtr<IMFMediaSource> mediaSource);
	~SourceReader();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFSourceReader>			GetSourceReader();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<SourceReaderRep> m_pRep = 0;
#pragma warning(pop)
};
