#pragma once
#include <windows.h>
#include <string>
#include <atlcomcli.h>

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

// This class is exported from the MediaFoundationTDD.dll

struct IMFMediaSession;
struct IMFTopology;

struct IMFAttributes;
struct IMFActivate;
struct IMFMediaSource;
struct IMFSourceReader;

class MediaFoundationTDDRep;
class MediaFoundationTDD_API MediaFoundationTDD
{
public:
	MediaFoundationTDD();
	virtual ~MediaFoundationTDD();

	HRESULT						GetLastHRESULT();

	void						CreateMediaSession();
	CComPtr<IMFMediaSession>	GetMediaSession();

	void						CreateTopology();
	CComPtr<IMFTopology>		GetTopology();

	void						CreateMediaSource(CComPtr<IMFActivate> myDevice);
	CComPtr<IMFMediaSource>		GetMediaSource();

	CComPtr<IMFActivate>		CreateVideoDevice(std::wstring videoDeviceName);
	CComPtr<IMFActivate>		CreateAudioDevice(std::wstring audioDeviceName);

private:
	MediaFoundationTDDRep* m_pRep = 0;
};