#include "XOSMFSourceReaderCallback.h"

#include "XOSMFAVSourceReader.h"
#include "XOSMFUtilities.h"

XOSMFSourceReaderCallback::XOSMFSourceReaderCallback(XOSMFAVSourceReader* pXOSMFAVSourceReader):
m_pXOSMFAVSourceReader(pXOSMFAVSourceReader),
m_nRefCount(0)
{
}

XOSMFSourceReaderCallback::~XOSMFSourceReaderCallback()
{
}

HRESULT XOSMFSourceReaderCallback::OnEvent(DWORD dwStreamIndex, IMFMediaEvent* pEvent)
{
	MediaEventType mt = MEUnknown;
	HRESULT hr = pEvent->GetType(&mt);
	if (SUCCEEDED_XOSb(hr))
	{
		if (mt != MESourceCharacteristicsChanged)
		{
			WCHAR mess[1024];
			swprintf_s(mess, 1024, L"XOSMFAVSourceReader::OnEvent stream(%d) type(%d)\n", dwStreamIndex, mt);
			OutputDebugStringW(mess);
		}
	}

	return S_OK;
}
HRESULT XOSMFSourceReaderCallback::OnFlush(DWORD dwStreamIndex)
{
	if (m_pXOSMFAVSourceReader)
	{
		return m_pXOSMFAVSourceReader->OnFlush(dwStreamIndex);
	}
	return E_FAIL;
}

HRESULT XOSMFSourceReaderCallback::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimeStamp, IMFSample* pSample)
{
	if (m_pXOSMFAVSourceReader)
	{
		return m_pXOSMFAVSourceReader->OnReadSample(hrStatus, dwStreamIndex, dwStreamFlags, llTimeStamp, pSample);
	}
	return E_FAIL;
}

HRESULT XOSMFSourceReaderCallback::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit [] =
	{
		{ &__uuidof(IMFSourceReaderCallback), ((INT) (INT_PTR) (static_cast<IMFSourceReaderCallback*>((XOSMFSourceReaderCallback*) 8)) - 8) },
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

ULONG XOSMFSourceReaderCallback::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}

ULONG XOSMFSourceReaderCallback::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}