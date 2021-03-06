#pragma once

#include <windows.h>
#include <atlcomcli.h>
#include <memory>

struct IMFActivate;
struct IMFMediaSource;
struct IMFMediaType;
class MediaSourceRep;
class MediaSource
{
public:
	MediaSource(CComPtr<IMFActivate> singleDevice);
	MediaSource(CComPtr<IMFActivate> videoDevice, CComPtr<IMFActivate> audioDevice);
	~MediaSource();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaSource>				GetMediaSource();
	CComPtr<IMFMediaType>				GetVideoMediaType();
	CComPtr<IMFMediaType>				GetAudioMediaType();
	void								SetCurrentMediaTypes();

private:
	MediaSource();
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<MediaSourceRep> m_pRep = 0;
#pragma warning(pop)
};
