#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>

struct IMFMediaSink;
struct IMFStreamSink;
class MediaSource;
class FileSinkRep;
class FileSink
{
public:
	FileSink(LPCWSTR fullFilePath, std::shared_ptr<MediaSource> aggregateMediaSource);
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
