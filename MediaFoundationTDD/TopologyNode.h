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
struct IMFTransform;
struct IMFMediaType;
class FileSink;
class TopologyNodeRep;
class MediaFoundationTDD_API TopologyNode
{
public:
	TopologyNode(std::wstring name);
	TopologyNode(std::wstring name, CComPtr<IMFTransform> transform);
	TopologyNode(std::wstring name, CComPtr<IMFMediaType> prefMediaType, CComPtr<IMFActivate> renderDevice);
	TopologyNode(std::wstring name, CComPtr<IMFTopologyNode> node);
	TopologyNode
	(
		std::wstring name,
		CComPtr<IMFMediaSource> mediaSource,
		CComPtr<IMFPresentationDescriptor> presentationDescriptor,
		CComPtr<IMFStreamDescriptor> streamDescriptor,
		CComPtr<IMFActivate> renderer
	);
	TopologyNode(std::wstring name, std::shared_ptr<FileSink> mediaSink);
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