#pragma once

#include <windows.h>
#include <atlcomcli.h>
#include <memory>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFActivate;
struct IMFMediaSource;
class MediaSourceRep;
class MediaFoundationTDD_API MediaSource
{
public:
	MediaSource(CComPtr<IMFActivate> singleDevice);
	MediaSource(CComPtr<IMFActivate> videoDevice, CComPtr<IMFActivate> audioDevice);
	~MediaSource();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaSource>				GetMediaSource();

private:
	MediaSource();
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<MediaSourceRep> m_pRep = 0;
#pragma warning(pop)
};
