#include "XOSMFSinkWriterCallback.h"

XOSMFSinkWriterCallback::XOSMFSinkWriterCallback() :
m_nRefCount(0)
{

}
XOSMFSinkWriterCallback::~XOSMFSinkWriterCallback()
{

}

HRESULT XOSMFSinkWriterCallback::OnFinalize(HRESULT hrStatus)
{
	WCHAR mess[1024];
	swprintf_s(mess, 1024, L"TSFileWriter::OnFinalize stream(0x%x)\n", hrStatus);
	OutputDebugStringW(mess);

	return S_OK;
}

HRESULT XOSMFSinkWriterCallback::OnMarker(DWORD dwStreamIndex, LPVOID pvContext)
{
	WCHAR mess[1024];
	swprintf_s(mess, 1024, L"TSFileWriter::OnMarker stream(%d) context(0x%I64x)\n", dwStreamIndex, (unsigned long long)pvContext);
	OutputDebugStringW(mess);

	return S_OK;
}

HRESULT XOSMFSinkWriterCallback::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit [] =
	{
		{ &__uuidof(IMFSourceReaderCallback), ((INT) (INT_PTR) (static_cast<IMFSinkWriterCallback*>((XOSMFSinkWriterCallback*) 8)) - 8) },
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

ULONG XOSMFSinkWriterCallback::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}

ULONG XOSMFSinkWriterCallback::Release()
{
	ULONG uCount = InterlockedDecrement(&m_nRefCount);
	if (uCount == 0)
	{
		delete this;
	}
	return uCount;
}
