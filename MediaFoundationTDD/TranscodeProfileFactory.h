#pragma once

#include <windows.h>
#include <atlcomcli.h>
#include <memory>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFTranscodeProfile;
struct IMFAttributes;
class TranscodeProfileFactoryRep;
class MediaFoundationTDD_API TranscodeProfileFactory
{
public:
	TranscodeProfileFactory();
	~TranscodeProfileFactory();

	HRESULT								GetLastHRESULT();

	CComPtr<IMFTranscodeProfile>		CreateVideoOnlyTranscodeProfile(CComPtr<IMFAttributes> videoInputAttributes);
	CComPtr<IMFTranscodeProfile>		CreateAudioOnlyTranscodeProfile();
private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::unique_ptr<TranscodeProfileFactoryRep> m_pRep = 0;
#pragma warning(pop)
};
