#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>
#include <string>

struct IMFMediaSession;
struct IMFTopology;
struct IMFActivate;
struct IMFMediaEvent;
class TopologyRep;
class MediaSource;
class Topology
{
public:
	Topology();
	Topology(CComPtr<IMFMediaEvent> mediaEvent);
	~Topology();

	HRESULT GetLastHRESULT();

	void CreateTopology
	(
		std::shared_ptr<MediaSource> captureSource,
		const std::wstring& fileToWrite,
		CComPtr<IMFActivate> videoRendererDevice,
		CComPtr<IMFActivate> audioRendererDevice,
		CComPtr<IMFMediaSession> mediaSession
	);
	void DumpTopology(CComPtr<IMFTopology> topology);

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<TopologyRep> m_pRep = 0;
#pragma warning(pop)
};
