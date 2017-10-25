#include "FileSink.h"
#include "MFUtils.h"
#include "AttributesFactory.h"
#include "MediaTypeFactory.h"
#include "MediaSource.h"
#include "PresentationDescriptor.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

class FileSinkRep : public MFUtils
{
public:
	FileSinkRep(LPCWSTR fullFilePath, std::shared_ptr<MediaSource> videoSource);
	~FileSinkRep();

	HRESULT					GetLastHRESULT();

	CComPtr<IMFMediaSink>	GetMediaSink();
	CComPtr<IMFStreamSink>	GetAudioStreamSink();
	CComPtr<IMFStreamSink>	GetVideoStreamSink();

private:
	CComPtr<IMFSinkWriter>	mSinkWriter = NULL;
	CComPtr<IMFMediaSink>	mMediaSink = NULL;
	DWORD mVideoStreamIndex = 0;
	CComPtr<IMFStreamSink>	mVideoStreamSink = NULL;
	DWORD mAudioStreamIndex = 1;
	CComPtr<IMFStreamSink>	mAudioStreamSink = NULL;
};
FileSink::FileSink(LPCWSTR fullFilePath, std::shared_ptr<MediaSource> videoSource)
{
	m_pRep = std::unique_ptr<FileSinkRep>(new FileSinkRep(fullFilePath, videoSource));
}
FileSinkRep::FileSinkRep(LPCWSTR fullFilePath, std::shared_ptr<MediaSource> videoSource)
{
	bool addStreamInWriter = false;
	AttributesFactory attributesFactory;
	CComPtr<IMFAttributes> attributes = attributesFactory.CreateFSinkAttrs();
	if (attributes)
	{
		CComPtr<IMFByteStream> outputByteStream = NULL;
		OnERR_return(MFCreateFile(MF_ACCESSMODE_WRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE, fullFilePath, &outputByteStream));
		OnERR_return(MFCreateMuxSink(MFStreamFormat_MPEG2Transport, NULL, outputByteStream, &mMediaSink));

		//OnERR_return(MFCreateSinkWriterFromURL(fullFilePath, NULL, attributes, &mSinkWriter));

		//// GetServiceForStream only works for .ts, .aac and .mp4 both fail with "The object does not support the specified service" c00d36ba MF_E_UNSUPPORTED_SERVICE
		//OnERR_return(mSinkWriter->GetServiceForStream(MF_SINK_WRITER_MEDIASINK, GUID_NULL, IID_IMFMediaSink, (LPVOID*)&mMediaSink));
		MediaTypeFactory mediaTypeFactory;
		//if (addStreamInWriter)
		//{
		//	OnERR_return(mSinkWriter->AddStream(mediaTypeFactory.CreateAudioEncodingMediaType(), &mAudioStreamIndex));
		//	OnERR_return(mMediaSink->GetStreamSinkByIndex(mAudioStreamIndex, &mAudioStreamSink));
		//}
		//else
		//{
			CComPtr<IMFMediaType> sourceVideoMediaType = videoSource->GetVideoMediaType();
			OnERR_return(mMediaSink->AddStreamSink(mVideoStreamIndex, mediaTypeFactory.CreateVideoEncodingMediaType(sourceVideoMediaType), &mVideoStreamSink));
			//OnERR_return(mMediaSink->AddStreamSink(mAudioStreamIndex, mediaTypeFactory.CreateAudioEncodingMediaType(), &mAudioStreamSink));
		//}
	}
}
FileSink::~FileSink()
{
}
FileSinkRep::~FileSinkRep()
{
}

HRESULT FileSink::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT FileSinkRep::GetLastHRESULT()
{
	return MFUtils::GetLastHRESULT();
}

CComPtr<IMFMediaSink> FileSink::GetMediaSink()
{
	return m_pRep->GetMediaSink();
}
CComPtr<IMFMediaSink> FileSinkRep::GetMediaSink()
{
	return mMediaSink;
}

CComPtr<IMFStreamSink> FileSink::GetAudioStreamSink()
{
	return m_pRep->GetAudioStreamSink();
}
CComPtr<IMFStreamSink> FileSinkRep::GetAudioStreamSink()
{
	if (!mAudioStreamSink)
	{
		OnERR_return_NULL(mMediaSink->GetStreamSinkByIndex(mAudioStreamIndex, &mAudioStreamSink));
	}
	return mAudioStreamSink;
}

CComPtr<IMFStreamSink> FileSink::GetVideoStreamSink()
{
	return m_pRep->GetVideoStreamSink();
}
CComPtr<IMFStreamSink> FileSinkRep::GetVideoStreamSink()
{
	if (!mVideoStreamSink)
	{
		OnERR_return_NULL(mMediaSink->GetStreamSinkByIndex(mVideoStreamIndex, &mVideoStreamSink));
	}
	return mVideoStreamSink;
}