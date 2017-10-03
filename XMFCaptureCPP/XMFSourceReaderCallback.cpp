#include "stdafx.h"
#include "XMFSourceReaderCallback.h"
#include "XMFAVSourceReader.h"
#include "XMFUtilities.h"

XMFSourceReaderCallback::XMFSourceReaderCallback(XMFAVSourceReader* pXMFAVSourceReader):
m_pXMFAVSourceReader(pXMFAVSourceReader),
m_nRefCount(0)
{
}

XMFSourceReaderCallback::~XMFSourceReaderCallback()
{
}

HRESULT XMFSourceReaderCallback::OnEvent(DWORD dwStreamIndex, IMFMediaEvent* pEvent)
{
	MediaEventType mt = MEUnknown;
	HRESULT hr = pEvent->GetType(&mt);
	if (SUCCEEDED_Xb(hr))
	{
		if (mt != MESourceCharacteristicsChanged)
		{
			WCHAR mess[1024];
			swprintf_s(mess, 1024, L"XMFSourceReaderCallback::OnEvent stream(%d) type(%d)\n", dwStreamIndex, mt);
			OutputDebugStringW(mess);
		}
	}

	return S_OK;
}
HRESULT XMFSourceReaderCallback::OnFlush(DWORD dwStreamIndex)
{
	if (m_pXMFAVSourceReader)
	{
		return m_pXMFAVSourceReader->OnFlush(dwStreamIndex);
	}
	return E_FAIL;
}

HRESULT XMFSourceReaderCallback::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimeStamp, IMFSample* pSample)
{
	if (m_pXMFAVSourceReader)
	{
		return m_pXMFAVSourceReader->OnReadSample(hrStatus, dwStreamIndex, dwStreamFlags, llTimeStamp, pSample);
	}
	return E_FAIL;
}

HRESULT XMFSourceReaderCallback::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit [] =
	{
		QITABENT(XMFSourceReaderCallback, IMFSourceReaderCallback),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

ULONG XMFSourceReaderCallback::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}

ULONG XMFSourceReaderCallback::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}