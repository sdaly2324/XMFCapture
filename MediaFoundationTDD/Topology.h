#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSession;
struct IMFTopology;
struct IMFActivate;
class MediaSource;
class TopologyRep;
class MediaFoundationTDD_API Topology
{
public:
	Topology();
	~Topology();

	HRESULT								GetLastHRESULT();

	void								CreateAudioPassthroughTopology(std::shared_ptr<MediaSource> audioSource, CComPtr<IMFActivate> audioRenderer);
	void								CreateVideoPassthroughTopology(std::shared_ptr<MediaSource> videoSource, CComPtr<IMFActivate> videoRenderer);
	void								ResolveTopology();
	void								SetTopology(CComPtr<IMFMediaSession> mediaSession);

	CComPtr<IMFTopology>				GetTopology();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<TopologyRep> m_pRep = 0;
#pragma warning(pop)
};
