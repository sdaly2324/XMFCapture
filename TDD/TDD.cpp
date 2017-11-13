
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
		std::wstring	myVideoDeviceName = L"XI100DUSB-SDI Video";		//<-------------------Video device to test-----------------------------
		std::wstring	myAudioDeviceName = L"XI100DUSB-SDI Audio";		//<-------------------Audio device to test-----------------------------
		//std::wstring	myVideoDeviceName = L"XI100DUSB-HDMI Video";	//<-------------------Video device to test-----------------------------
		//std::wstring	myAudioDeviceName = L"XI100DUSB-HDMI Audio";	//<-------------------Audio device to test-----------------------------

		std::wstring	myCaptureFilePath = L"C:\\";					//<-------------------Path to file captures----------------------------
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
		TEST_METHOD(VideoAndAudioCaptureAndPassthrough)
		{
			//Sleep(5000);
			bool useAudio = true;
			bool useVideo = true;
			mVideoDisplayControl->SetIsParticipating(useVideo);

			// source
			std::shared_ptr<MediaSource> aggregateMediaSource = nullptr;
			if (!useAudio && useVideo)
			{
				aggregateMediaSource = std::make_shared<MediaSource>(mVideoCaptureDevice);
				Assert::AreEqual(aggregateMediaSource->GetLastHRESULT(), S_OK);
			}
			else if (useAudio && !useVideo)
			{
				aggregateMediaSource = std::make_shared<MediaSource>(mAudioCaptureDevice);
				Assert::AreEqual(aggregateMediaSource->GetLastHRESULT(), S_OK);
			}
			else if (useAudio && useVideo)
			{
				aggregateMediaSource = std::make_shared<MediaSource>(mVideoCaptureDevice, mAudioCaptureDevice);
				Assert::AreEqual(aggregateMediaSource->GetLastHRESULT(), S_OK);
			}
			else
			{
				return;
			}

			// Topology
			std::wstring fileToWrite = myCaptureFilePath + L"VideoAndAdudioCaptureAndPassthrough.ts";
			mTopology->CreateTopology(aggregateMediaSource, fileToWrite, mVideoRenderer, mAudioRenderer);
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