#pragma once
#include <windows.h>
#include <atlcomcli.h>
#include <memory>

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
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<PresentationDescriptorRep> m_pRep = 0;
#pragma warning(pop)
};
