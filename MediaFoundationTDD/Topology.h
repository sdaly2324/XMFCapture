#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>
#include <string>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSession;
struct IMFTopology;
struct IMFActivate;
struct IMFMediaSource;
struct IMFMediaEvent;
class FileSink;
class TopologyRep;
class MediaSource;
class MediaFoundationTDD_API Topology
{
public:
	Topology();
	Topology(CComPtr<IMFMediaEvent> mediaEvent);
	~Topology();

	HRESULT GetLastHRESULT();

	void CreatePassthroughTopology
	(
		CComPtr<IMFMediaSource> mediaSource,
		CComPtr<IMFActivate> videoRenderer,
		CComPtr<IMFActivate> audioRenderer
	);
	void CreateCaptureAndPassthroughTopology
	(
		CComPtr<IMFMediaSource> mediaSource,
		CComPtr<IMFActivate> videoRenderer,
		CComPtr<IMFActivate> audioRenderer,
		std::shared_ptr<FileSink> mediaSink
	);
	void CreateVideoOnlyCaptureTopology(std::shared_ptr<MediaSource> mediaSource, const std::wstring& fileToWrite);
	void CreateVideoOnlyCaptureAndPassthroughTopology
	(
		std::shared_ptr<MediaSource> mediaSource,
		const std::wstring& fileToWrite,
		CComPtr<IMFActivate> videoRendererDevice
	);
	void CreateVideoAndAudioCaptureAndPassthroughTopology
	(
		std::shared_ptr<MediaSource> videoMediaSource,
		std::shared_ptr<MediaSource> audioMediaSource,
		std::shared_ptr<MediaSource> aggregateMediaSource,
		const std::wstring& fileToWrite,
		CComPtr<IMFActivate> videoRendererDevice,
		CComPtr<IMFActivate> audioRendererDevice
	);

	void CreateAudioOnlyCaptureTopology(std::shared_ptr<MediaSource> mediaSource, const std::wstring& fileToWrite);

	void ResolveTopology();
	void SetTopology(CComPtr<IMFMediaSession> mediaSession);

	CComPtr<IMFTopology> GetTopology();
	void DumpTopology(CComPtr<IMFTopology> topology);

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<TopologyRep> m_pRep = 0;
#pragma warning(pop)
};
