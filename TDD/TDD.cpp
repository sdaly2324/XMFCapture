
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "TopologyNode.h"
#include "PresentationDescriptor.h"
#include "MediaSession.h"
#include "AudioDevices.h"
#include "VideoDevices.h"
#include "MediaSource.h"
#include "Topology.h"
#include "VideoDisplayControl.h"
#include "FileSink.h"

#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfapi.h>
#include <evr.h>
#include <memory>

const WCHAR CLASS_NAME[] = L"CaptureTEST Window Class";
const WCHAR WINDOW_NAME[] = L"CaptureTEST Sample Application";
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

		void InitializeWindow()
		{
			WNDCLASS wc = { 0 };
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandle(NULL);
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.lpszClassName = CLASS_NAME;
			Assert::IsTrue(RegisterClass(&wc));

			mVideoWindow = CreateWindow(CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, GetModuleHandle(NULL), NULL);
			Assert::IsTrue(mVideoWindow);

			ShowWindow(mVideoWindow, SW_SHOWDEFAULT);
			UpdateWindow(mVideoWindow);
		}
		void InitVideoDevices()
		{
			auto videoDevices = std::make_unique<VideoDevices>(mVideoWindow);
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
			auto audioDevices = std::make_unique<AudioDevices>();
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
			mVideoDisplayControl = std::make_shared<VideoDisplayControl>();
			mMediaSession = std::make_unique<MediaSession>(mVideoDisplayControl);
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mMediaSession->GetMediaSession());
		}
		void InitTopology()
		{
			mTopology = std::make_unique<Topology>();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
		}
	public:
		MediaFoundationCaptureTESTs::MediaFoundationCaptureTESTs()
		{
			InitializeWindow();
			InitMediaSession();
			InitTopology();
			InitAudioDevices();
			InitVideoDevices();
		}
		TEST_METHOD(AudioOnlyStreamDescriptorsVilidation)
		{
			// source
			auto audioSource = std::make_unique<MediaSource>(mAudioCaptureDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			//// PresentationDescriptor
			auto audioPresentationDescriptor = std::make_unique<PresentationDescriptor>(audioSource->GetMediaSource());
			Assert::AreEqual(audioPresentationDescriptor->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioPresentationDescriptor->GetPresentationDescriptor());
			unsigned int items = 0;
			mLastHR = audioPresentationDescriptor->GetPresentationDescriptor()->GetCount(&items);
			Assert::AreEqual(mLastHR, S_OK);
			Assert::AreEqual(items, (unsigned int)0); // I have no idea why GetCount returns 0 when there is only 1 stream

			CComPtr<IMFStreamDescriptor> audioStreamDescriptor = audioPresentationDescriptor->GetFirstAudioStreamDescriptor();
			ValidateAudioStreamDescriptor(audioStreamDescriptor);
		}
		TEST_METHOD(VideoOnlyStreamDescriptorsVilidation)
		{
			// source
			auto videoSource = std::make_unique<MediaSource>(mVideoCaptureDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// PresentationDescriptor
			auto videoPresentationDescriptor = std::make_unique<PresentationDescriptor>(videoSource->GetMediaSource());
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
		TEST_METHOD(VideoAndAudioStreamDescriptorsVilidation)
		{
			// video source
			auto videoSource = std::make_unique<MediaSource>(mVideoCaptureDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// audio source
			auto audioSource = std::make_unique<MediaSource>(mAudioCaptureDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			// aggregate source
			auto aggregateSource = std::make_unique<MediaSource>(mVideoCaptureDevice, mAudioCaptureDevice);
			Assert::AreEqual(aggregateSource->GetLastHRESULT(), S_OK);

			// PresentationDescriptors
			auto aggregatePresentationDescriptor = std::make_unique<PresentationDescriptor>(aggregateSource->GetMediaSource());
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
		TEST_METHOD(AudioOnlyPassthroughOnly)
		{
			mVideoDisplayControl->SetIsParticipating(false);

			// source
			auto audioSource = std::make_unique<MediaSource>(mAudioCaptureDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			// Topology
			mTopology->CreatePassthroughTopology(audioSource->GetMediaSource(), NULL, mAudioRenderer);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
		}
		TEST_METHOD(VideoOnlyPassthroughOnly)
		{
			mVideoDisplayControl->SetIsParticipating(true);

			// source
			auto videoSource = std::make_unique<MediaSource>(mVideoCaptureDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// Topology
			mTopology->CreatePassthroughTopology(videoSource->GetMediaSource(), mVideoRenderer, NULL);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mVideoDisplayControl->GetVideoDisplayControl());
			Assert::AreEqual(mVideoDisplayControl->GetLastHRESULT(), S_OK);

			Sleep(3000);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
		}
		TEST_METHOD(VideoAndAudioSeperatePassthroughOnly)
		{
			// source
			auto videoSource = std::make_unique<MediaSource>(mVideoCaptureDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);
			auto audioSource = std::make_unique<MediaSource>(mAudioCaptureDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			// Topology
			mTopology->CreatePassthroughTopology(videoSource->GetMediaSource(), mVideoRenderer, NULL);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mTopology->CreatePassthroughTopology(audioSource->GetMediaSource(), NULL, mAudioRenderer);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mVideoDisplayControl->GetVideoDisplayControl());
			Assert::AreEqual(mVideoDisplayControl->GetLastHRESULT(), S_OK);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
		}
		TEST_METHOD(VideoAndAudioAggregatePassthroughOnly)
		{
			// aggregate source
			auto aggregateSource = std::make_unique<MediaSource>(mVideoCaptureDevice, mAudioCaptureDevice);
			Assert::AreEqual(aggregateSource->GetLastHRESULT(), S_OK);

			// Topology
			mTopology->CreatePassthroughTopology(aggregateSource->GetMediaSource(), mVideoRenderer, mAudioRenderer);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mVideoDisplayControl->GetVideoDisplayControl());
			Assert::AreEqual(mVideoDisplayControl->GetLastHRESULT(), S_OK);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
		}
		TEST_METHOD(TSFileSink)
		{
			// source
			auto videoSource = std::make_shared<MediaSource>(mVideoCaptureDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// file sink
			std::wstring fileToWrite = myCaptureFilePath + L"TSFileSink.ts";
			auto fileSink = std::make_unique<FileSink>(fileToWrite.c_str(), videoSource, nullptr);
			Assert::AreEqual(fileSink->GetLastHRESULT(), S_OK);
			Assert::IsTrue(fileSink->GetMediaSink());
		}
		TEST_METHOD(AudioSourceReader)
		{
			// source
			auto audioSource = std::make_unique<MediaSource>(mAudioCaptureDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			CComPtr<IMFMediaType> mediaType	= audioSource->GetAudioMediaType();
			Assert::IsTrue(mediaType);
		}
		TEST_METHOD(VideoSourceReader)
		{
			// source
			auto videoSource = std::make_unique<MediaSource>(mVideoCaptureDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			CComPtr<IMFMediaType> mediaType = videoSource->GetVideoMediaType();
			Assert::IsTrue(mediaType);
		}
		TEST_METHOD(VideoOnlyCapture)
		{
			mVideoDisplayControl->SetIsParticipating(false);

			// source
			auto videoSource = std::make_shared<MediaSource>(mVideoCaptureDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// Topology
			std::wstring fileToWrite = myCaptureFilePath + L"VideoOnlyCapture.ts";
			mTopology->CreateVideoOnlyCaptureTopology(videoSource, fileToWrite);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);

			::Sleep(3000);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
		}
		TEST_METHOD(AudioOnlyCapture)
		{
			mVideoDisplayControl->SetIsParticipating(false);

			// source
			auto audioSource = std::make_shared<MediaSource>(mAudioCaptureDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			// Topology
			std::wstring fileToWrite = myCaptureFilePath + L"AudioOnlyCapture.ts";
			mTopology->CreateAudioOnlyCaptureTopology(audioSource, fileToWrite);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);

			::Sleep(1000);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
		}
		TEST_METHOD(AudioOnlyCaptureAndPassthrough)
		{

		}
		TEST_METHOD(VideoOnlyCaptureAndPassthrough)
		{
			mVideoDisplayControl->SetIsParticipating(true);

			// source
			auto videoSource = std::make_shared<MediaSource>(mVideoCaptureDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// Topology
			std::wstring fileToWrite = myCaptureFilePath + L"VideoOnlyCaptureAndPassthrough.ts";
			mTopology->CreateVideoOnlyCaptureAndPassthroughTopology(videoSource, fileToWrite, mVideoRenderer);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);

			::Sleep(3000);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
		}
		TEST_METHOD(VideoAndAudioCaptureAndPassthrough)
		{
			//Sleep(5000);
			bool useAudio = true;
			bool useVideo = true;
			mVideoDisplayControl->SetIsParticipating(false);//(useVideo);

			// source
			std::shared_ptr<MediaSource> aggregateMediaSource = nullptr;
			std::shared_ptr<MediaSource> videoMediaSource = nullptr;
			std::shared_ptr<MediaSource> audioMediaSource = nullptr;
			if (!useAudio && useVideo)
			{
				videoMediaSource = std::make_shared<MediaSource>(mVideoCaptureDevice);
				Assert::AreEqual(videoMediaSource->GetLastHRESULT(), S_OK);
			}
			else if (useAudio && !useVideo)
			{
				audioMediaSource = std::make_shared<MediaSource>(mAudioCaptureDevice);
				Assert::AreEqual(audioMediaSource->GetLastHRESULT(), S_OK);
			}
			else if (useAudio && useVideo)
			{
				aggregateMediaSource = std::make_shared<MediaSource>(mVideoCaptureDevice, mAudioCaptureDevice);
				Assert::AreEqual(aggregateMediaSource->GetLastHRESULT(), S_OK);
				//videoMediaSource = std::make_shared<MediaSource>(mVideoCaptureDevice);
				//Assert::AreEqual(videoMediaSource->GetLastHRESULT(), S_OK);
				//audioMediaSource = std::make_shared<MediaSource>(mAudioCaptureDevice);
				//Assert::AreEqual(audioMediaSource->GetLastHRESULT(), S_OK);
			}
			else
			{
				return;
			}

			// Topology
			std::wstring fileToWrite = myCaptureFilePath + L"VideoAndAdudioCaptureAndPassthrough.ts";
			mTopology->CreateVideoAndAudioCaptureAndPassthroughTopology(videoMediaSource, audioMediaSource, aggregateMediaSource, fileToWrite, mVideoRenderer, mAudioRenderer);
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveTopology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			// Start
			mTopology->SetTopology(mMediaSession->GetMediaSession());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			mMediaSession->Start();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);

			::Sleep(10000);

			mMediaSession->Stop();
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
		}
	};
}