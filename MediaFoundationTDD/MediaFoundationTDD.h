#pragma once
#include <windows.h>
#include <string>

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

	HRESULT				GetLastHRESULT();

	void				CreateMediaSession();
	IMFMediaSession*	GetMediaSession();

	void				CreateTopology();
	IMFTopology*		GetTopology();

	void				CreateMediaSource(IMFActivate* myDevice);
	IMFMediaSource*		GetMediaSource();

	void				CreateSourceReader(IMFMediaSource* mediaSource, IMFAttributes* sourceReaderAsycCallbackAttributes);
	IMFSourceReader*	GetSourceReader();

	IMFActivate*		CreateVideoOnlyDevice(std::wstring videoDeviceName);

private:
	MediaFoundationTDDRep* m_pRep = 0;
};