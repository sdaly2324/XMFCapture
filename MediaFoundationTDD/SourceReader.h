#pragma once
#include <windows.h>
#include <atlcomcli.h>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSource;
struct IMFSourceReader;
struct IMFPresentationDescriptor;
class SourceReaderRep;
class MediaFoundationTDD_API SourceReader
{
public:
	SourceReader(CComPtr<IMFMediaSource> mediaSource);
	~SourceReader();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFSourceReader>			GetSourceReader();
	CComPtr<IMFPresentationDescriptor>	GetPresentationDescriptor();

private:
	SourceReaderRep* m_pRep = 0;
};
