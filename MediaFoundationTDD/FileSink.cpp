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
	FileSinkRep(LPCWSTR fullFilePath, std::shared_ptr<MediaSource> aggregateMediaSource);
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
	DWORD mAudioStreamIndex = 0;
	CComPtr<IMFStreamSink>	mAudioStreamSink = NULL;
};
FileSink::FileSink(LPCWSTR fullFilePath, std::shared_ptr<MediaSource> aggregateMediaSource)
{
	m_pRep = std::unique_ptr<FileSinkRep>(new FileSinkRep(fullFilePath, aggregateMediaSource));
}
FileSinkRep::FileSinkRep(LPCWSTR fullFilePath, std::shared_ptr<MediaSource> aggregateMediaSource)
{
	AttributesFactory attributesFactory;
	CComPtr<IMFAttributes> attributes = attributesFactory.CreateFileSinkAttrs();
	if (attributes)
	{
		CComPtr<IMFByteStream> outputByteStream = NULL;
		OnERR_return(MFCreateFile(MF_ACCESSMODE_WRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE, fullFilePath, &outputByteStream));
		OnERR_return(MFCreateMuxSink(MFStreamFormat_MPEG2Transport, attributes, outputByteStream, &mMediaSink));

		MediaTypeFactory mediaTypeFactory;
		DWORD newStreamIndex = 0;

		CComPtr<IMFAttributes> sourceVideoMediaAttrs = aggregateMediaSource->GetVideoMediaType();
		if (sourceVideoMediaAttrs)
		{
			OnERR_return(mMediaSink->AddStreamSink(newStreamIndex, mediaTypeFactory.CreateVideoEncodingMediaType(sourceVideoMediaAttrs), &mVideoStreamSink));
			mVideoStreamIndex = newStreamIndex;
			newStreamIndex++;
		}

		CComPtr<IMFAttributes> sourceAudioMediaAttrs = aggregateMediaSource->GetAudioMediaType();
		if (sourceAudioMediaAttrs)
		{
			OnERR_return(mMediaSink->AddStreamSink(newStreamIndex, mediaTypeFactory.CreateAudioEncodingMediaType(), &mAudioStreamSink));
			mAudioStreamIndex = newStreamIndex;
			newStreamIndex++;
		}
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