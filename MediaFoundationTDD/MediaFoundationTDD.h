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

	void						CreateTopology();
	CComPtr<IMFTopology>		GetTopology();

	CComPtr<IMFMediaSource>		CreateMediaSource(CComPtr<IMFActivate> myDevice);
	CComPtr<IMFMediaSource>		CreateAggregateMediaSource(CComPtr<IMFMediaSource> pVideoSource, CComPtr<IMFMediaSource> pAudioSource);

private:
	MediaFoundationTDDRep* m_pRep = 0;
};