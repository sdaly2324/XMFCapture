#pragma once

#include <windows.h>
#include <atlcomcli.h>

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
	MediaSourceRep* m_pRep = 0;
};
