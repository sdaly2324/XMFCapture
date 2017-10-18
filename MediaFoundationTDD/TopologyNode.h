#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <string>
#include <memory>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSource;
struct IMFTopologyNode;
struct IMFActivate;
class TopologyNodeRep;
class MediaFoundationTDD_API TopologyNode
{
public:
	TopologyNode(CComPtr<IMFMediaSource> mediaSource);
	TopologyNode(CComPtr<IMFActivate> device);
	~TopologyNode();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopologyNode>			GetTopologyNode();

private:
	TopologyNode();
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<TopologyNodeRep> m_pRep = 0;
#pragma warning(pop)
};