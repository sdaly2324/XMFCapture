#include "XMFSinkWriterCallback.h"

XMFSinkWriterCallback::XMFSinkWriterCallback() :
m_nRefCount(0)
{

}
XMFSinkWriterCallback::~XMFSinkWriterCallback()
{

}

HRESULT XMFSinkWriterCallback::OnFinalize(HRESULT hrStatus)
{
	WCHAR mess[1024];
	swprintf_s(mess, 1024, L"TSFileWriter::OnFinalize stream(0x%x)\n", hrStatus);
	OutputDebugStringW(mess);

	return S_OK;
}

HRESULT XMFSinkWriterCallback::OnMarker(DWORD dwStreamIndex, LPVOID pvContext)
{
	WCHAR mess[1024];
	swprintf_s(mess, 1024, L"TSFileWriter::OnMarker stream(%d) context(0x%I64x)\n", dwStreamIndex, (unsigned long long)pvContext);
	OutputDebugStringW(mess);

	return S_OK;
}

HRESULT XMFSinkWriterCallback::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit [] =
	{
		{ &__uuidof(IMFSourceReaderCallback), ((INT) (INT_PTR) (static_cast<IMFSinkWriterCallback*>((XMFSinkWriterCallback*) 8)) - 8) },
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

ULONG XMFSinkWriterCallback::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}

ULONG XMFSinkWriterCallback::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}
