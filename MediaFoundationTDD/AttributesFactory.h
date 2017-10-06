#pragma once
#include <windows.h>
#include <atlcomcli.h>
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
	AttributesFactoryRep* m_pRep = 0;
};