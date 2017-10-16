#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFAttributes;
class AttributesFactoryRep;
class MediaFoundationTDD_API AttributesFactory
{
public:
	AttributesFactory();
	~AttributesFactory();

	HRESULT GetLastHRESULT();

	CComPtr<IMFAttributes>	CreateVideoDeviceAttributes();
	CComPtr<IMFAttributes>	CreateAudioDeviceAttributes();
	CComPtr<IMFAttributes>	CreateSourceReaderAsycCallbackAttributes(IUnknown* callBack);

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<AttributesFactoryRep> m_pRep = 0;
#pragma warning(pop)
};