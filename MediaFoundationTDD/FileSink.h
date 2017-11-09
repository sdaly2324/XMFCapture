#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSink;
struct IMFStreamSink;
class MediaSource;
class FileSinkRep;
class MediaFoundationTDD_API FileSink
{
public:
	FileSink(LPCWSTR fullFilePath, std::shared_ptr<MediaSource> videoMediaSource, std::shared_ptr<MediaSource> audioMediaSource);
	~FileSink();

	HRESULT							GetLastHRESULT();

	CComPtr<IMFMediaSink>			GetMediaSink();
	CComPtr<IMFStreamSink>			GetAudioStreamSink();
	CComPtr<IMFStreamSink>			GetVideoStreamSink();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<FileSinkRep> m_pRep = 0;
#pragma warning(pop)
};
