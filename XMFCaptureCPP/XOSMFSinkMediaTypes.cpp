#include "XOSMFSinkMediaTypes.h"
#include "XOSMFUtilities.h"

#include <codecapi.h>

HRESULT CopyAttribute(CComPtr<IMFMediaType> pSrc, CComPtr<IMFMediaType> pDest, const GUID& key)
{
	PROPVARIANT var;
	PropVariantInit(&var);

	HRESULT hr = S_OK;

	hr = pSrc->GetItem(key, &var);
	if (SUCCEEDED_XOSb(hr))
	{
		hr = pDest->SetItem(key, var);
	}

	PropVariantClear(&var);
	return hr;
}

XOSMFSinkMediaTypeBase::XOSMFSinkMediaTypeBase() :
m_pMediaType(NULL)
{
	HRESULT hr = MFCreateMediaType(&m_pMediaType);
	if (FAILED(hr))
	{
		m_pMediaType = NULL;
		DUMPHr_XOSv(hr, L"MFCreateMediaType failed");
	}
}
XOSMFSinkMediaTypeBase::~XOSMFSinkMediaTypeBase()
{

}

CComPtr<IMFMediaType> XOSMFSinkMediaTypeBase::GetMediaType()
{
	return m_pMediaType;
}

HRESULT XOSMFSinkMediaTypeBase::ConfigFromSource(CComPtr<IMFMediaType> pSourceMT)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		hr = CheckSource(pSourceMT);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = CopyRequiredAttrsFromSource(pSourceMT);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = SetCoreMTAttrs();
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = SetEncoderMTAttrs();
	}

	return hr;
}

/*****Video******/

XOSMFVideoSinkMediaType::XOSMFVideoSinkMediaType()
{
}
XOSMFVideoSinkMediaType::~XOSMFVideoSinkMediaType()
{
}

HRESULT XOSMFVideoSinkMediaType::CopyRequiredAttrsFromSource(CComPtr<IMFMediaType> pSourceMT)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		hr = CopyAttribute(pSourceMT, m_pMediaType, MF_MT_FRAME_SIZE);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = CopyAttribute(pSourceMT, m_pMediaType, MF_MT_FRAME_RATE);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = CopyAttribute(pSourceMT, m_pMediaType, MF_MT_INTERLACE_MODE);
	}

	return hr;
}
HRESULT XOSMFVideoSinkMediaType::SetCoreMTAttrs()
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		hr = m_pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = m_pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
		//hr = m_pMediaType->SetUINT32(MF_MT_MAX_KEYFRAME_SPACING, 30);
		//hr = m_pMediaType->SetUINT32(CODECAPI_AVEncMPVGOPSize, 30);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = m_pMediaType->SetUINT32(MF_MT_AVG_BITRATE, 6157744);
		//hr = m_pMediaType->SetUINT32(MF_MT_AVG_BITRATE, 3078872);
		//hr = m_pMediaType->SetUINT32(MF_MT_AVG_BITRATE, 1539436);
		//hr = m_pMediaType->SetUINT32(MF_MT_AVG_BITRATE, 769718);
		//hr = m_pMediaType->SetUINT32(MF_MT_AVG_BITRATE, 384859);
	}

	return hr;
}

HRESULT XOSMFVideoSinkMediaType::SetEncoderMTAttrs()
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		hr = m_pMediaType->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_High);
		//hr = m_pMediaType->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_Simple);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = m_pMediaType->SetUINT32(MF_MT_MPEG2_LEVEL, eAVEncH264VLevel4_1);
		//hr = m_pMediaType->SetUINT32(MF_MT_MPEG2_LEVEL, eAVEncH264VLevel3_2);
	}

	return hr;
}

HRESULT XOSMFVideoSinkMediaType::CheckSource(CComPtr<IMFMediaType> pSourceMT)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		GUID Mtype = { 0 };
		hr = pSourceMT->GetGUID(MF_MT_MAJOR_TYPE, &Mtype);
		if (MFMediaType_Video != Mtype)
		{
			hr = E_INVALIDARG;
		}
	}

	return hr;
}

/*****Audio******/
XOSMFAudioSinkMediaType::XOSMFAudioSinkMediaType()
{
}
XOSMFAudioSinkMediaType::~XOSMFAudioSinkMediaType()
{
}

HRESULT XOSMFAudioSinkMediaType::CopyRequiredAttrsFromSource(CComPtr<IMFMediaType> /*pSourceMT*/)
{
	return S_OK;
}

HRESULT XOSMFAudioSinkMediaType::SetCoreMTAttrs()
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		hr = m_pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = m_pMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, SINK_AUDIO_SAMPLE_RATE);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = m_pMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
	}

	return hr;
}

HRESULT SetPCM(CComPtr<IMFMediaType> pSinkMT)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		hr = pSinkMT->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = pSinkMT->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = pSinkMT->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 4 * SINK_AUDIO_SAMPLE_RATE);
	}

	if (SUCCEEDED_XOSb(hr))
	{
		hr = pSinkMT->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 4);
	}

	return hr;
}
HRESULT SetAAC(CComPtr<IMFMediaType> pSinkMT)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		hr = pSinkMT->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
	}

	return hr;
}
HRESULT XOSMFAudioSinkMediaType::SetEncoderMTAttrs()
{
	return SetAAC(m_pMediaType);
	//return SetPCM(m_pMediaType);
}

HRESULT XOSMFAudioSinkMediaType::CheckSource(CComPtr<IMFMediaType> pSourceMT)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED_XOSb(hr))
	{
		GUID Mtype = { 0 };
		hr = pSourceMT->GetGUID(MF_MT_MAJOR_TYPE, &Mtype);
		if (MFMediaType_Audio != Mtype)
		{
			hr = E_INVALIDARG;
		}
	}

	return hr;
}