#pragma once

#include <memory>
#include <windows.h>
#include <string>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

class XMFFormatRep;
struct IMFActivate;
class XMFFormat
{
public:
	XMFFormat(CComPtr<IMFActivate> deviceThatOwnsThisList, DWORD streamID, DWORD m_FormatIndex);
	virtual ~XMFFormat();

	void AddAtribute(GUID attr, std::wstring val);

	DWORD GetStreamID();
	DWORD GetFormatIndex();
	bool operator==(const XMFFormat& obj);
	bool CantainsAllAttributes(const XMFFormat& obj);

private:
	XMFFormatRep* m_RepPtr;
};