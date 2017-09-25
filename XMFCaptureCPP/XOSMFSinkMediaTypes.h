#pragma once

#include <windows.h>
#include <string>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <atlbase.h>

struct IMFMediaType;

class XOSMFSinkMediaTypeBase
{
public:
	XOSMFSinkMediaTypeBase();
	virtual ~XOSMFSinkMediaTypeBase();

	CComPtr<IMFMediaType> GetMediaType();
	HRESULT ConfigFromSource(CComPtr<IMFMediaType> pSourceMT);

protected:
	virtual HRESULT CopyRequiredAttrsFromSource(CComPtr<IMFMediaType> pSourceMT) = 0;
	virtual HRESULT SetCoreMTAttrs() = 0;
	virtual HRESULT SetEncoderMTAttrs() = 0;

	CComPtr<IMFMediaType> m_pMediaType;

private:
	virtual HRESULT CheckSource(CComPtr<IMFMediaType> pSourceMT) = 0;
};

class XOSMFVideoSinkMediaType : public XOSMFSinkMediaTypeBase
{
public:
	XOSMFVideoSinkMediaType();
	virtual ~XOSMFVideoSinkMediaType();

protected:
private:
	virtual HRESULT CopyRequiredAttrsFromSource(CComPtr<IMFMediaType> pSourceMT);
	virtual HRESULT SetCoreMTAttrs();
	virtual HRESULT SetEncoderMTAttrs();
	virtual HRESULT CheckSource(CComPtr<IMFMediaType> pSourceMT);
};

class XOSMFAudioSinkMediaType : public XOSMFSinkMediaTypeBase
{
public:
	XOSMFAudioSinkMediaType();
	virtual ~XOSMFAudioSinkMediaType();

protected:
private:
	virtual HRESULT CopyRequiredAttrsFromSource(CComPtr<IMFMediaType> pSourceMT);
	virtual HRESULT SetCoreMTAttrs();
	virtual HRESULT SetEncoderMTAttrs();
	virtual HRESULT CheckSource(CComPtr<IMFMediaType> pSourceMT);
};

#define SINK_AUDIO_SAMPLE_RATE 48000