#include "stdafx.h"
#include "XMFUtilities.h"
#include "XMFAttrToStrHelper.h"

#include <mfapi.h>
#include <mfidl.h>

void DumpAttrImp(CComPtr<IMFAttributes> attrs, const std::wstring& typeName, const std::wstring& name)
{
	UINT32 pcItems = 0;
	HRESULT hr = attrs->GetCount(&pcItems);
	if (SUCCEEDED_Xb(hr))
	{
		for (unsigned int x = 0; x < pcItems; x++)
		{
			GUID pguidKey;
			PROPVARIANT pValue;
			hr = attrs->GetItemByIndex(x, &pguidKey, &pValue);
			if (SUCCEEDED_Xb(hr))
			{
				// get attribute name
				std::wstring attrName = XMFAttrToStrHelper::GUIDToAttrName(pguidKey);
				std::wstring attrVal = L"UNKOWN";

				attrVal = XMFAttrToStrHelper::PropToValue(attrs, pguidKey, pValue);

				wchar_t  mess[1024];
				swprintf_s(mess, 1024, L"%s (%s) ATTR %.3d %-56s %s\n", typeName.c_str(), name.c_str(), x, attrName.c_str(), attrVal.c_str());
				OutputDebugStringW(mess);
			}
		}
	}
}

bool XOSStringsAreTheSame(XOSString left, XOSString right)
{
	return *left == *right;
}