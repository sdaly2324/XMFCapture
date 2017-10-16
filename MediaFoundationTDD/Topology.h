#pragma once
#include <windows.h>
#include <atlcomcli.h>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSession;
struct IMFTopology;
class MediaSource;
class TopologyRep;
class MediaFoundationTDD_API Topology
{
public:
	Topology();
	~Topology();

	HRESULT								GetLastHRESULT();

	void								CreateAudioPassthroughTopology(MediaSource* audioSource);
	void								CreateVideoPassthroughTopology(MediaSource* videoSource, HWND windowForVideo);
	void								ResolveSingleSourceTopology();
	void								SetTopology(CComPtr<IMFMediaSession> mediaSession);

	CComPtr<IMFTopology>				GetTopology();

private:
	TopologyRep* m_pRep = 0;
};
