#include "AttributesFactory.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>


AttributesFactory::AttributesFactory()
{	
}
AttributesFactory::~AttributesFactory()
{
}

HRESULT AttributesFactory::GetLastHRESULT()
{
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFAttributes> AttributesFactory::CreateVideoDeviceAttrs()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 1));
	OnERR_return_NULL(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID));
	OnERR_return_NULL(retVal->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateAudioDeviceAttrs()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 1));
	OnERR_return_NULL(retVal->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID));
	OnERR_return_NULL(retVal->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateFileSinkAttrs()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 0));
	//OnERR_return_NULL(attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	//OnERR_return_NULL(attributes->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Stream));
	//OnERR_return_NULL(attributes->SetGUID(MF_MT_SUBTYPE, MFStreamFormat_MPEG2Transport));
	OnERR_return_NULL(retVal->SetGUID(MF_TRANSCODE_CONTAINERTYPE, MFTranscodeContainerType_MPEG2));
	OnERR_return_NULL(retVal->SetUINT32(MF_TRANSCODE_ADJUST_PROFILE, MF_TRANSCODE_ADJUST_PROFILE_DEFAULT));
	OnERR_return_NULL(retVal->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateAudioOutAttrs()
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 0));
	OnERR_return_NULL(retVal->SetUINT32(MF_LOW_LATENCY, TRUE));
	OnERR_return_NULL(retVal->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
	return retVal;
}

HRESULT AttributesFactory::CopyAttribute(CComPtr<IMFAttributes> sourceAttribute, CComPtr<IMFAttributes> destinationAttribute, const GUID& attributeGUID)
{
	PROPVARIANT var;
	PropVariantInit(&var);
	HRESULT hr = sourceAttribute->GetItem(attributeGUID, &var);
	if (SUCCEEDED(hr))
	{
		hr = destinationAttribute->SetItem(attributeGUID, var);
		PropVariantClear(&var);
	}
	return hr;
}


CComPtr<IMFAttributes> AttributesFactory::CreateVideoEncodeAttrs(CComPtr<IMFAttributes> videoInputAttributes)
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 0));
	if (!videoInputAttributes)
	{
		// assume 720p5994
		OnERR_return_NULL(retVal->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		OnERR_return_NULL(retVal->SetUINT64(MF_MT_FRAME_SIZE, 5497558139600));
		OnERR_return_NULL(retVal->SetUINT64(MF_MT_FRAME_RATE, 257698037761001));
		OnERR_return_NULL(retVal->SetUINT64(MF_MT_PIXEL_ASPECT_RATIO, 4294967297));
		OnERR_return_NULL(retVal->SetUINT32(MF_MT_INTERLACE_MODE, 2));
	}
	else
	{
		OnERR_return_NULL(videoInputAttributes->CopyAllItems(retVal));
	}

	OnERR_return_NULL(retVal->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
	OnERR_return_NULL(retVal->SetUINT32(MF_MT_VIDEO_PROFILE, 100));
	OnERR_return_NULL(retVal->SetUINT32(MF_MT_VIDEO_LEVEL, 41));
	//OnERR_return_NULL(retVal->SetUINT32(MF_MT_MAX_KEYFRAME_SPACING, 30)); // FIXED IN XMFSinkWriterRep::BeginWriting // DOES NOT WORK WITH IMFCaptureEngine
	OnERR_return_NULL(retVal->SetUINT32(MF_MT_AVG_BITRATE, 6000000)); // 6 megabits
	OnERR_return_NULL(retVal->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));

	return retVal;
}

CComPtr<IMFAttributes> AttributesFactory::CreateVideoNV12Attrs(CComPtr<IMFAttributes> videoInputAttributes)
{
	CComPtr<IMFAttributes> retVal = NULL;
	OnERR_return_NULL(MFCreateAttributes(&retVal, 0));
	if (!videoInputAttributes)
	{
		// assume 720p5994
		OnERR_return_NULL(retVal->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		OnERR_return_NULL(retVal->SetUINT64(MF_MT_FRAME_SIZE, 5497558139600));
		OnERR_return_NULL(retVal->SetUINT64(MF_MT_FRAME_RATE, 257698037761001));
		OnERR_return_NULL(retVal->SetUINT64(MF_MT_PIXEL_ASPECT_RATIO, 4294967297));
		OnERR_return_NULL(retVal->SetUINT32(MF_MT_INTERLACE_MODE, 2));
	}
	else
	{
		OnERR_return_NULL(videoInputAttributes->CopyAllItems(retVal));
	}
	OnERR_return_NULL(retVal->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12));
	return retVal;
}