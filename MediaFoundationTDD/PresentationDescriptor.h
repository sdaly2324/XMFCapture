#pragma once
#include <windows.h>
#include <atlcomcli.h>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFStreamDescriptor;
struct IMFMediaSource;
struct IMFPresentationDescriptor;
class PresentationDescriptorRep;
class MediaFoundationTDD_API PresentationDescriptor
{
public:
	PresentationDescriptor(CComPtr<IMFMediaSource> mediaSource);
	~PresentationDescriptor();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFPresentationDescriptor>	GetPresentationDescriptor();
	CComPtr<IMFStreamDescriptor>		GetFirstVideoStreamDescriptor();
	CComPtr<IMFStreamDescriptor>		GetFirstAudioStreamDescriptor();

private:
	PresentationDescriptor();
	PresentationDescriptorRep* m_pRep = 0;
};
