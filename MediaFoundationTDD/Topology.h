#pragma once
#include <windows.h>
#include <atlcomcli.h>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFTopology;
class TopologyRep;
class MediaFoundationTDD_API Topology
{
public:
	Topology();
	~Topology();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopology>				GetTopology();

private:
	TopologyRep* m_pRep = 0;
};
