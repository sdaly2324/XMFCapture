#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>

struct IMFAttributes;
class AttributesFactoryRep;
class AttributesFactory
{
public:
	AttributesFactory();
	~AttributesFactory();

	HRESULT GetLastHRESULT();

	CComPtr<IMFAttributes>	CreateVideoDeviceAttrs();
	CComPtr<IMFAttributes>	CreateAudioDeviceAttrs();
	CComPtr<IMFAttributes>	CreateSInkReaderCbAttrs(IUnknown* callBack);
	CComPtr<IMFAttributes>	CreateFileSinkAttrs();
	CComPtr<IMFAttributes>	CreateVideoNV12Attrs(CComPtr<IMFAttributes> videoInputAttributes);
	CComPtr<IMFAttributes>	CreateVideoEncodeAttrs(CComPtr<IMFAttributes> videoInputAttributes);
	CComPtr<IMFAttributes>	CreateAudioOutAttrs();

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<AttributesFactoryRep> m_pRep = 0;
#pragma warning(pop)
};