
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "SourceReader.h"
#include "TopologyNode.h"
#include "PresentationDescriptor.h"
#include "MediaSession.h"
#include "AudioDevices.h"
#include "VideoDevices.h"
#include "MediaSource.h"
#include "Topology.h"
#include "VideoDisplayControl.h"
#include "SinkWriter.h"

#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfapi.h>
#include <evr.h>
#include <memory>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ERASEBKGND:
		return 1;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

namespace MediaFoundationTesing
{		
	TEST_CLASS(MediaFoundationCaptureTESTs)
	{
	private:
		std::wstring	myVideoDeviceName = L"XI100DUSB-SDI Video";								//<-------------------Video device to test-----------------------------
		std::wstring	myAudioDeviceName = L"Digital Audio Interface (XI100DUSB-SDI Audio)";	//<-------------------Audio device to test-----------------------------
		std::wstring	myCaptureFilePath = L"C:\\";											//<-------------------Path to file captures----------------------------
		HRESULT			mLastHR = S_OK;
		std::unique_ptr<MediaSession>	mMediaSession = NULL;
		std::unique_ptr<Topology>		mTopology = NULL;
		std::shared_ptr<VideoDisplayControl> mVideoDisplayControl = NULL;
		CComPtr<IMFActivate> mAudioCaptureDevice = NULL;
		CComPtr<IMFActivate> mVideoCaptureDevice = NULL;
		CComPtr<IMFActivate> mAudioRenderer = NULL;
		CComPtr<IMFActivate> mVideoRenderer = NULL;
		HWND mVideoWindow = NULL;

		void InitVideoDevices()
		{
			std::unique_ptr<VideoDevices> videoDevices(new VideoDevices(NULL));
			Assert::AreEqual(videoDevices->GetLastHRESULT(), S_OK);

			mVideoCaptureDevice = videoDevices->GetCaptureVideoDevice(myVideoDeviceName);
			Assert::AreEqual(videoDevices->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mVideoCaptureDevice);

			mVideoRenderer = videoDevices->GetVideoRenderer();
			Assert::AreEqual(videoDevices->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mVideoRenderer);
		}
		void InitAudioDevices()
		{
			std::unique_ptr<AudioDevices> audioDevices(new AudioDevices());
			Assert::AreEqual(audioDevices->GetLastHRESULT(), S_OK);

			mAudioCaptureDevice = audioDevices->GetCaptureAudioDevice(myAudioDeviceName);
			Assert::AreEqual(audioDevices->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mAudioCaptureDevice);

			mAudioRenderer = audioDevices->GetAudioRenderer();
			Assert::AreEqual(audioDevices->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mAudioRenderer);
		}
		void ValidateVideoStreamDescriptor(CComPtr<IMFStreamDescriptor> videoStreamDescriptor)
		{
			return ValidateStreamDescriptor(videoStreamDescriptor, MFMediaType_Video);
		}
		void ValidateAudioStreamDescriptor(CComPtr<IMFStreamDescriptor> audioStreamDescriptor)
		{
			return ValidateStreamDescriptor(audioStreamDescriptor, MFMediaType_Audio);
		}
		void ValidateStreamDescriptor(CComPtr<IMFStreamDescriptor> streamDescriptor, GUID MAJOR_TYPE)
		{
			CComPtr<IMFMediaTypeHandler> mediaTypeHandler = NULL;
			mLastHR = streamDescriptor->GetMediaTypeHandler(&mediaTypeHandler);
			Assert::AreEqual(mLastHR, S_OK);
			unsigned long mediaTypes = 0;
			mLastHR = mediaTypeHandler->GetMediaTypeCount(&mediaTypes);
			Assert::AreEqual(mLastHR, S_OK);
			Assert::IsTrue(mediaTypes > 0);
			for (unsigned int mediaType = 0; mediaType < mediaTypes; mediaType++)
			{
				CComPtr<IMFMediaType> mediaTypeAPI = NULL;
				mLastHR = mediaTypeHandler->GetMediaTypeByIndex(mediaType, &mediaTypeAPI);
				Assert::AreEqual(mLastHR, S_OK);
				GUID guidValue = GUID_NULL;
				mLastHR = mediaTypeAPI->GetGUID(MF_MT_MAJOR_TYPE, &guidValue);
				Assert::AreEqual(mLastHR, S_OK);
				Assert::IsTrue(guidValue == MAJOR_TYPE);
			}
		}
		void InitMediaSession()
		{
			mVideoDisplayControl = std::shared_ptr<VideoDisplayControl>(new VideoDisplayControl());
			mMediaSession = std::unique_ptr<MediaSession>(new MediaSession(mVideoDisplayControl));
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mMediaSession->GetMediaSession());
		}
		void InitTopology()
		{
			mTopology = std::unique_ptr<Topology>(new Topology());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mTopology->GetTopology());
		}
	public:
		MediaFoundationCaptureTESTs::MediaFoundationCaptureTESTs()
		{
			InitMediaSession();
			InitTopology();
			InitAudioDevices();
			InitVideoDevices();
		}
		TEST_METHOD(StreamDescriptorsAudioOnlyVilidation)
		{
			// source
			std::unique_ptr<MediaSource> audioSource(new MediaSource(mAudioCaptureDevice));
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			//// PresentationDescriptor
			std::unique_ptr<PresentationDescriptor> audioPresentationDescriptor(new PresentationDescriptor(audioSource->GetMediaSource()));
			Assert::AreEqual(audioPresentationDescriptor->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioPresentationDescriptor->GetPresentationDescriptor());
			unsigned int items = 0;
			mLastHR = audioPresentationDescriptor->GetPresentationDescriptor()->GetCount(&items);
			Assert::AreEqual(mLastHR, S_OK);
			Assert::AreEqual(items, (unsigned int)0); // I have no idea why GetCount returns 0 when there is only 1 stream

			CComPtr<IMFStreamDescriptor> audioStreamDescriptor = audioPresentationDescriptor->GetFirstAudioStreamDescriptor();
			ValidateAudioStreamDescriptor(audioStreamDescriptor);
		}
		TEST_METHOD(StreamDescriptorsVideoOnlyVilidation)
		{
			// source
			std::unique_ptr<MediaSource> videoSource(new MediaSource(mVideoCaptureDevice));
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// PresentationDescriptor
			std::unique_ptr<PresentationDescriptor> videoPresentationDescriptor(new PresentationDescriptor(videoSource->GetMediaSource()));
			Assert::AreEqual(videoPresentationDescriptor->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoPresentationDescriptor->GetPresentationDescriptor());
			unsigned int items = 0;
			mLastHR = videoPresentationDescriptor->GetPresentationDescriptor()->GetCount(&items);
			Assert::AreEqual(mLastHR, S_OK);
			Assert::AreEqual(items, (unsigned int)0); // I have no idea why GetCount returns 0 when there is only 1 stream

			// StreamDescriptors
			CComPtr<IMFStreamDescriptor> videoStreamDescriptor = videoPresentationDescriptor->GetFirstVideoStreamDescriptor();
			ValidateVideoStreamDescriptor(videoStreamDescriptor);
		}
		TEST_METHOD(StreamDescriptorsAggregateVilidation)
		{
			// video source
			std::unique_ptr<MediaSource> videoSource(new MediaSource(mVideoCaptureDevice));
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// audio source
			std::unique_ptr<MediaSource> audioSource(new MediaSource(mAudioCaptureDevice));
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			// aggregate source
			std::unique_ptr<MediaSource> aggregateSource(new MediaSource(mVideoCaptureDevice, mAudioCaptureDevice));
			Assert::AreEqual(aggregateSource->GetLastHRESULT(), S_OK);

			// PresentationDescriptors
			std::unique_ptr<PresentationDescriptor> aggregatePresentationDescriptor(new PresentationDescriptor(aggregateSource->GetMediaSource()));
			Assert::AreEqual(aggregatePresentationDescriptor->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregatePresentationDescriptor->GetPresentationDescriptor());
			unsigned int items = 0;
			mLastHR = aggregatePresentationDescriptor->GetPresentationDescriptor()->GetCount(&items);
			Assert::AreEqual(mLastHR, S_OK);
			Assert::AreEqual(items, (unsigned int)2);

			// StreamDescriptors
			CComPtr<IMFStreamDescriptor> videoStreamDescriptor = aggregatePresentationDescriptor->GetFirstVideoStreamDescriptor();
			ValidateVideoStreamDescriptor(videoStreamDescriptor);

			CComPtr<IMFStreamDescriptor> audioStreamDescriptor = aggregatePresentationDescriptor->GetFirstAudioStreamDescriptor();
			ValidateAudioStreamDescriptor(audioStreamDescriptor);
		}
		TEST_METHOD(PassthroughAudioOnlyTopology)
		{
			// source
			std::shared_ptr<MediaSource> audioSource(new MediaSource(mAudioCaptureDevice));
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			// Topology
			mTopology->CreateAudioPassthroughTopology(audioSource , mAudioRenderer);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);

			::Sleep(1000);
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			::Sleep(1000);
		}
		TEST_METHOD(PassthroughVideoOnlyTopology)
		{
			// source
			std::shared_ptr<MediaSource> videoSource(new MediaSource(mVideoCaptureDevice));
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// Topology
			mTopology->CreateVideoPassthroughTopology(videoSource, mVideoRenderer);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);

			::Sleep(1000);
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mVideoDisplayControl->GetVideoDisplayControl());
			Assert::AreEqual(mVideoDisplayControl->GetLastHRESULT(), S_OK);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			::Sleep(1000);
		}
		TEST_METHOD(PassthroughVideoAndAudioTopology)
		{
			// source
			std::shared_ptr<MediaSource> videoSource(new MediaSource(mVideoCaptureDevice));
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);
			std::shared_ptr<MediaSource> audioSource(new MediaSource(mAudioCaptureDevice));
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			// Topology
			mTopology->CreateVideoPassthroughTopology(videoSource, mVideoRenderer);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mTopology->CreateAudioPassthroughTopology(audioSource, mAudioRenderer);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);

			::Sleep(1000);
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mVideoDisplayControl->GetVideoDisplayControl());
			Assert::AreEqual(mVideoDisplayControl->GetLastHRESULT(), S_OK);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			::Sleep(1000);
		}
		TEST_METHOD(CreateAACFileSink)
		{
			// sink writer
			std::wstring fileToWrite = myCaptureFilePath + L"audioOnly.aac";
			std::shared_ptr<SinkWriter> sinkWriter(new SinkWriter(fileToWrite.c_str()));
			Assert::AreEqual(sinkWriter->GetLastHRESULT(), S_OK);
			Assert::IsTrue(sinkWriter->GetSinkWriter());
		}
		TEST_METHOD(CreateTSFileSink)
		{
			// sink writer
			std::wstring fileToWrite = myCaptureFilePath + L"capture.ts";
			std::shared_ptr<SinkWriter> sinkWriter(new SinkWriter(fileToWrite.c_str()));
			Assert::AreEqual(sinkWriter->GetLastHRESULT(), S_OK);
			Assert::IsTrue(sinkWriter->GetSinkWriter());
		}
		TEST_METHOD(CreateAudioSourceReader)
		{
			// source
			std::shared_ptr<MediaSource> audioSource(new MediaSource(mAudioCaptureDevice));
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			// source reader
			std::unique_ptr<SourceReader> audioSourceReader(new SourceReader(audioSource->GetMediaSource()));
			Assert::AreEqual(audioSourceReader->GetLastHRESULT(), S_OK);
		}
		TEST_METHOD(CreateVideoSourceReader)
		{
			// source
			std::shared_ptr<MediaSource> videoSource(new MediaSource(mVideoCaptureDevice));
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// source reader
			std::unique_ptr<SourceReader> videoSourceReader(new SourceReader(videoSource->GetMediaSource()));
			Assert::AreEqual(videoSourceReader->GetLastHRESULT(), S_OK);
		}
	};
}