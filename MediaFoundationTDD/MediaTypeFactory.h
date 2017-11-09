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
struct IMFMediaType;
struct IMFAttributes;
class MediaTypeFactoryRep;
class MediaFoundationTDD_API MediaTypeFactory
{
public:
	MediaTypeFactory();
	~MediaTypeFactory();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaType>				CreateAudioEncodingMediaType();
	CComPtr<IMFMediaType>				CreateAudioInputMediaType();
	CComPtr<IMFMediaType>				CreateVideoEncodingMediaType(CComPtr<IMFAttributes> inAttrs);

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<MediaTypeFactoryRep> m_pRep = 0;
#pragma warning(pop)
};
