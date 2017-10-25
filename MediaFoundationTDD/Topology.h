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
class FileSink;
class TopologyRep;
class MediaSource;
class MediaFoundationTDD_API Topology
{
public:
	Topology();
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
	void CreateAudioOnlyCaptureTopology(std::shared_ptr<MediaSource> mediaSource, const std::wstring& fileToWrite);
	void ResolveTopology();
	void SetTopology(CComPtr<IMFMediaSession> mediaSession);

	CComPtr<IMFTopology> GetTopology();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<TopologyRep> m_pRep = 0;
#pragma warning(pop)
};
