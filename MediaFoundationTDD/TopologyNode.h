#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <string>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFMediaSource;
struct IMFTopologyNode;
class TopologyNodeRep;
class MediaFoundationTDD_API TopologyNode
{
public:
	TopologyNode(CComPtr<IMFMediaSource> mediaSource);
	TopologyNode(HWND windowForVideo);
	TopologyNode(std::wstring nodeType);
	~TopologyNode();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopologyNode>			GetTopologyNode();

private:
	TopologyNode();
	TopologyNodeRep* m_pRep = 0;
};