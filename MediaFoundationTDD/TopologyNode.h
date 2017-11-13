#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <string>
#include <memory>

struct IMFMediaSource;
struct IMFTopologyNode;
struct IMFActivate;
struct IMFPresentationDescriptor;
struct IMFStreamDescriptor;
struct IMFTransform;
struct IMFMediaType;
struct IMFStreamSink;
class TopologyNodeRep;
class TopologyNode
{
public:
	TopologyNode(std::wstring name);
	TopologyNode(std::wstring name, CComPtr<IMFTransform> transform);
	TopologyNode(std::wstring name, CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice);
	TopologyNode(std::wstring name, CComPtr<IMFTopologyNode> node);
	TopologyNode(std::wstring name, CComPtr<IMFStreamSink> streamSink);
	TopologyNode
	(
		std::wstring name,
		CComPtr<IMFMediaSource> mediaSource,
		CComPtr<IMFPresentationDescriptor> presentationDescriptor,
		CComPtr<IMFStreamDescriptor> streamDescriptor,
		CComPtr<IMFActivate> renderer
	);
	~TopologyNode();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTopologyNode>			GetNode();
	CComPtr<IMFTopologyNode>			GetRendererNode();
	CComPtr<IMFMediaType>				GetOutputPrefType();
	CComPtr<IMFMediaType>				GetInputPrefType();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<TopologyNodeRep> m_pRep = 0;
#pragma warning(pop)
};