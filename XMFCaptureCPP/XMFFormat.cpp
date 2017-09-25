#include "XMFFormat.h"
#include "XMFUtilities.h" // for GUID key

#include <map>
#include <mfobjects.h>

#define ATTRIBUTE const GUID, const std::wstring
#define ATTRIBUTES std::map<ATTRIBUTE>

class XMFFormatRep
{
public:
	XMFFormatRep(CComPtr<IMFActivate> deviceThatOwnsThisList, DWORD streamID, DWORD m_FormatIndex);
	virtual ~XMFFormatRep();

	void AddAtribute(GUID attr, std::wstring val);

	DWORD GetStreamID();
	DWORD GetFormatIndex();
	bool operator==(const XMFFormatRep& obj);
	bool CantainsAllAttributes(const XMFFormatRep& obj);

private:
	CComPtr<IMFActivate>	m_pDeviceThatOwnsThisList;
	DWORD			m_StreamID;
	DWORD			m_FormatIndex;
	ATTRIBUTES		m_AttributeList;
};

XMFFormat::XMFFormat(CComPtr<IMFActivate> deviceThatOwnsThisList, DWORD streamID, DWORD m_FormatIndex)
{
	m_RepPtr = new XMFFormatRep(deviceThatOwnsThisList, streamID, m_FormatIndex);
}
XMFFormatRep::XMFFormatRep(CComPtr<IMFActivate> deviceThatOwnsThisList, DWORD streamID, DWORD m_FormatIndex) :
m_pDeviceThatOwnsThisList(deviceThatOwnsThisList),
m_StreamID(streamID),
m_FormatIndex(m_FormatIndex)
{
	m_AttributeList.clear();
}

XMFFormat::~XMFFormat()
{
	if (m_RepPtr)
	{
		delete m_RepPtr;
		m_RepPtr = NULL;
	}
}
XMFFormatRep::~XMFFormatRep()
{
	m_AttributeList.clear();
}

void XMFFormat::AddAtribute(GUID attr, std::wstring val)
{
	if (m_RepPtr)
	{
		m_RepPtr->AddAtribute(attr, val);
	}
}
void XMFFormatRep::AddAtribute(GUID attr, std::wstring val)
{
	m_AttributeList.insert(std::pair<ATTRIBUTE>(attr, val));
}

DWORD XMFFormat::GetStreamID()
{
	if (m_RepPtr)
	{
		return m_RepPtr->GetStreamID();
	}
	return 0;
}
DWORD XMFFormatRep::GetStreamID()
{
	return m_StreamID;
}

DWORD XMFFormat::GetFormatIndex()
{
	if (m_RepPtr)
	{
		return m_RepPtr->GetFormatIndex();
	}
	return 0;
}
DWORD XMFFormatRep::GetFormatIndex()
{
	return m_FormatIndex;
}

bool XMFFormat::operator==(const XMFFormat& obj)
{
	if (m_RepPtr)
	{
		return m_RepPtr->operator==(*(obj.m_RepPtr));
	}
	return false;
}
bool XMFFormatRep::operator==(const XMFFormatRep& obj)
{
	if (m_pDeviceThatOwnsThisList == obj.m_pDeviceThatOwnsThisList &&
		m_StreamID == obj.m_StreamID &&
		m_FormatIndex == obj.m_FormatIndex)
	{
		// deep compare m_AttributeList ???
		return true;
	}
	return false;
}

bool XMFFormat::CantainsAllAttributes(const XMFFormat& givenFormat)
{
	if (m_RepPtr)
	{
		return m_RepPtr->CantainsAllAttributes(*(givenFormat.m_RepPtr));
	}
	return false;
}
bool XMFFormatRep::CantainsAllAttributes(const XMFFormatRep& givenFormat)
{
	bool fullMatch = true;
	for (auto& anAttributeWeAreLookingFor : givenFormat.m_AttributeList)
	{
		if (m_AttributeList.count(anAttributeWeAreLookingFor.first))
		{
			if (_wcsicmp(anAttributeWeAreLookingFor.second.c_str(), m_AttributeList[anAttributeWeAreLookingFor.first].c_str()) != 0)
			{
				fullMatch = false;
				break;
			}
		}
		else
		{
			fullMatch = false;
			break;
		}
	}
	return fullMatch;
}