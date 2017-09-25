#pragma once

#include <memory>
#include <comdef.h>
#include <atlcomcli.h>

#include "XMFUtilities.h"

struct IMFTopology;
class XMFTopoBuilderRep;
class XMFAggregateSourceReader;
class XMFTopoBuilder
{
public:
	XMFTopoBuilder(std::shared_ptr<XMFAggregateSourceReader> pAggregateSource, XOSString outputPath);
	virtual ~XMFTopoBuilder();

    // create a topology for the URL that will be rendered in the specified window
    HRESULT RenderURL(PCWSTR sURL, HWND videoHwnd);

    // get the created topology
	HRESULT GetTopology(CComPtr<IMFTopology>& apIMFTopology);

    // shutdown the media source for the topology
    HRESULT ShutdownSource();

private:
	XMFTopoBuilderRep* m_RepPtr;
};
