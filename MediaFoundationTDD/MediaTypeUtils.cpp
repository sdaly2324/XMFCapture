#include "MediaTypeUtils.h"
#include "AttributesFactory.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <evr.h>
#include <d3d9.h>
#include <Dxva2api.h>

MediaTypeUtils::MediaTypeUtils()
{

}
void MediaTypeUtils::DumpAvailableAACFormats(CComPtr<IMFCollection> availableTypes)
{
	// DUMP available AAC formats
	DWORD count = 0;
	HRESULT hr = availableTypes->GetElementCount(&count);
	for (DWORD i = 0; i < count; i++)
	{
		CComPtr<IMFMediaType> pMediaType = NULL;
		hr = GetCollectionObject(availableTypes, i, &pMediaType);
		WCHAR count[1024];
		swprintf_s(count, 1024, L"%d", i);
		MFUtils::DumpAttr(pMediaType, L"AUDIO AAC", count);
		OutputDebugStringW(L"\n");
	}
}

HRESULT MediaTypeUtils::GetLastHRESULT()
{
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFMediaType> MediaTypeUtils::CreateAudioInputMediaType()
{
	CComPtr<IMFAttributes> attributes = NULL;
	OnERR_return_NULL(MFCreateAttributes(&attributes, 0));
	OnERR_return_NULL(attributes->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
	OnERR_return_NULL(attributes->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
	OnERR_return_NULL(attributes->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16));
	OnERR_return_NULL(attributes->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 48000));
	OnERR_return_NULL(attributes->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2));

	CComPtr<IMFMediaType> retVal = NULL;
	OnERR_return_NULL(MFCreateMediaType(&retVal));
	OnERR_return_NULL(attributes->CopyAllItems(retVal));
	return retVal;
}

CComPtr<IMFMediaType> MediaTypeUtils::CreateAudioEncodingMediaType()
{
	CComPtr<IMFMediaType> retVal = NULL;
	AttributesFactory attributesFactory;
	CComPtr<IMFAttributes> attributes = attributesFactory.CreateAudioOutAttrs();
	if (attributes)
	{
		CComPtr<IMFCollection> availableTypes = NULL;
		OnERR_return_NULL(MFTranscodeGetAudioOutputAvailableTypes(MFAudioFormat_AAC, MFT_ENUM_FLAG_ALL | MFT_ENUM_FLAG_SORTANDFILTER, attributes, &availableTypes));
		
		//DumpAvailableAACFormats(availableTypes);
		DWORD formatIndexWeWantForOutput = 19;
		//	Audio AAC Output(19) ATTR 000 MF_MT_AUDIO_AVG_BYTES_PER_SECOND                         24000
		//	Audio AAC Output(19) ATTR 001 MF_MT_AVG_BITRATE                                        192000
		//	Audio AAC Output(19) ATTR 002 MF_MT_AUDIO_BLOCK_ALIGNMENT                              1
		//	Audio AAC Output(19) ATTR 003 MF_MT_AUDIO_NUM_CHANNELS                                 2
		//	Audio AAC Output(19) ATTR 004 MF_MT_COMPRESSED                                         1
		//	Audio AAC Output(19) ATTR 005 MF_MT_MAJOR_TYPE                                         MFMediaType_Audio
		//	Audio AAC Output(19) ATTR 006 MF_MT_AUDIO_SAMPLES_PER_SECOND                           48000
		//	Audio AAC Output(19) ATTR 007 MF_MT_AM_FORMAT_TYPE                                     FORMAT_WaveFormatEx
		//	Audio AAC Output(19) ATTR 008 MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION                 41
		//	Audio AAC Output(19) ATTR 009 MF_MT_AUDIO_PREFER_WAVEFORMATEX                          1
		//	Audio AAC Output(19) ATTR 010 MF_MT_USER_DATA                                          BLOB
		//	Audio AAC Output(19) ATTR 011 MF_MT_FIXED_SIZE_SAMPLES                                 0
		//	Audio AAC Output(19) ATTR 012 MF_MT_AAC_PAYLOAD_TYPE                                   0
		//	Audio AAC Output(19) ATTR 013 MF_MT_AUDIO_BITS_PER_SAMPLE                              16
		//	Audio AAC Output(19) ATTR 014 MF_MT_SUBTYPE                                            MFAudioFormat_AAC

		OnERR_return_NULL(GetCollectionObject(availableTypes, formatIndexWeWantForOutput, &retVal));
	}
	return retVal;
}

CComPtr<IMFMediaType> MediaTypeUtils::CreateVideoEncodingMediaType(CComPtr<IMFAttributes> inAttrs)
{
	AttributesFactory attributesFactory;
	CComPtr<IMFAttributes> outAttrs = attributesFactory.CreateVideoEncodeAttrs(inAttrs);
	if (!outAttrs)
	{
		SetLastHR_Fail();
		return NULL;
	}
	CComPtr<IMFMediaType> retVal = NULL;
	OnERR_return_NULL(MFCreateMediaType(&retVal));
	OnERR_return_NULL(outAttrs->CopyAllItems(retVal));
	return retVal;
}

CComPtr<IMFMediaType> MediaTypeUtils::CreateVideoNV12MediaType(CComPtr<IMFAttributes> inAttrs)
{
	AttributesFactory attributesFactory;
	CComPtr<IMFAttributes> outAttrs = attributesFactory.CreateVideoNV12Attrs(inAttrs);
	if (!outAttrs)
	{
		SetLastHR_Fail();
		return NULL;
	}
	CComPtr<IMFMediaType> retVal = NULL;
	OnERR_return_NULL(MFCreateMediaType(&retVal));
	OnERR_return_NULL(outAttrs->CopyAllItems(retVal));
	return retVal;
}

CaptureInputMode MediaTypeUtils::ConvertVideoMediaTypeToCaptureInputMode(CComPtr<IMFMediaType> mediaType)
{
	GUID guidValue;
	mediaType->GetGUID(MF_MT_MAJOR_TYPE, &guidValue);
	if (guidValue != MFMediaType_Video)
	{
		return captureInputModeUnknown;
	}

	UINT32 width = 0;
	UINT32 height = 0;
	if (IsHRError(MFGetAttributeRatio(mediaType, MF_MT_FRAME_SIZE, &width, &height)))
	{
		return captureInputModeUnknown;
	}

	UINT32 frameRatenumerator = 0;
	UINT32 frameRateDenominator = 0;
	if (IsHRError(MFGetAttributeRatio(mediaType, MF_MT_FRAME_RATE, &frameRatenumerator, &frameRateDenominator)))
	{
		return captureInputModeUnknown;
	}

	UINT32 interLacedMode;
	bool progressive = false;
	if (IsHRError(mediaType->GetUINT32(MF_MT_INTERLACE_MODE, &interLacedMode)))
	{
		return captureInputModeUnknown;
	}
	if (interLacedMode == 2)
	{
		progressive = true;
	}

	if (frameRatenumerator == 60000 && frameRateDenominator == 1001)
	{// 59.94
		if (width == 1280 && height == 720)
		{// 720
			return captureInputModeHD720p5994;
		}
		else if (width == 1920 && height == 1080)
		{// 1080
			if (progressive)
			{
				return captureInputModeHD1080p5994;
			}
			else
			{
				return captureInputModeHD1080i5994;
			}
		}
	}
	if (frameRatenumerator == 60 && frameRateDenominator == 1)
	{// 60
		if (width == 1280 && height == 720)
		{// 720
			return captureInputModeHD720p60;
		}
		else if (width == 1920 && height == 1080)
		{// 1080
			if (progressive)
			{
				return captureInputModeHD1080p6000;
			}
			else
			{
				return captureInputModeHD1080i6000;
			}
		}
	}
	if (frameRatenumerator == 50 && frameRateDenominator == 1)
	{// 50
		if (width == 1280 && height == 720)
		{// 720
			return captureInputModeHD720p50;
		}
		else if (width == 1920 && height == 1080)
		{// 1080
			if (progressive)
			{
				return captureInputModeHD1080p50;
			}
			else
			{
				return captureInputModeHD1080i50;
			}
		}
	}
	if (frameRatenumerator == 30000 && frameRateDenominator == 1001)
	{// 29.976
		if (progressive)
		{
			return captureInputModeNTSCp;
		}
		else
		{
			return captureInputModeNTSC;
		}
	}
	if (frameRatenumerator == 25 && frameRateDenominator == 1)
	{// 25
		if (progressive)
		{
			return captureInputModePALp;
		}
		else
		{
			return captureInputModePAL;
		}
	}

	return captureInputModeUnknown;
}