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
class MediaTypeFactoryRep;
class MediaFoundationTDD_API MediaTypeFactory
{
public:
	MediaTypeFactory();
	~MediaTypeFactory();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFMediaType>				CreateVideoEncodingMediaType(CComPtr<IMFMediaType> vInMType);
	CComPtr<IMFMediaType>				CreateAudioEncodingMediaType();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<MediaTypeFactoryRep> m_pRep = 0;
#pragma warning(pop)
};
