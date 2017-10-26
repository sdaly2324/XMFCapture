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
struct IMFPresentationDescriptor;
struct IMFStreamDescriptor;
class FileSink;
class TopologyNodeRep;
class MediaFoundationTDD_API TopologyNode
{
public:
	TopologyNode();
	TopologyNode(CComPtr<IMFActivate> renderDevice);
	TopologyNode
	(
		CComPtr<IMFMediaSource> mediaSource,
		CComPtr<IMFPresentationDescriptor> presentationDescriptor,
		CComPtr<IMFStreamDescriptor> streamDescriptor,
		CComPtr<IMFActivate> renderer
	);
	TopologyNode(std::shared_ptr<FileSink> mediaSink);
	~TopologyNode();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopologyNode>			GetNode();
	CComPtr<IMFTopologyNode>			GetRendererNode();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<TopologyNodeRep> m_pRep = 0;
#pragma warning(pop)
};