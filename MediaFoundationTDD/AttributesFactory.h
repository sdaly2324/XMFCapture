#pragma once

#include "MFUtils.h"

#include <windows.h>
#include <atlcomcli.h>
#include <memory>

struct IMFAttributes;
class AttributesFactoryRep;
class AttributesFactory : public MFUtils
{
public:
	AttributesFactory();
	~AttributesFactory();

	static HRESULT GetLastHRESULT();

	static CComPtr<IMFAttributes>	CreateVideoDeviceAttrs();
	static CComPtr<IMFAttributes>	CreateAudioDeviceAttrs();
	static CComPtr<IMFAttributes>	CreateFileSinkAttrs();
	static CComPtr<IMFAttributes>	CreateVideoNV12Attrs(CComPtr<IMFAttributes> videoInputAttributes);
	static CComPtr<IMFAttributes>	CreateVideoEncodeAttrs(CComPtr<IMFAttributes> videoInputAttributes);
	static CComPtr<IMFAttributes>	CreateAudioOutAttrs();
private:
	static HRESULT CopyAttribute(CComPtr<IMFAttributes> sourceAttribute, CComPtr<IMFAttributes> destinationAttribute, const GUID& attributeGUID);
};