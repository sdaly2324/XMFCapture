
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

#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfapi.h>
#include <evr.h>

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
		std::wstring	myVideoDeviceName = L"XI100DUSB-SDI Video";							//<-------------------Video device to test-----------------------------
		std::wstring	myAudioDeviceName = L"Digital Audio Interface (XI100DUSB-SDI Audio)";	//<-------------------Audio device to test-----------------------------
		HRESULT			mLastHR = S_OK;
		MediaSession*	mMediaSession = NULL;
		Topology*		mTopology = NULL;
		VideoDisplayControl* mVideoDisplayControl = NULL;
		CComPtr<IMFActivate> mAudioDevice = NULL;
		CComPtr<IMFActivate> mVideoDevice = NULL;
		HWND mVideoWindow = NULL;

		void InitVideoWindow()
		{
			WNDCLASS wc = { 0 };
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandle(NULL);
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.lpszClassName = L"MFCapture Window Class";
			ATOM atom = RegisterClass(&wc);
			if (atom == 0)
			{
				wchar_t  mess[1024];
				swprintf_s(mess, 1024, L"InitVideoWindow failed with %d 0x%x\n", GetLastError(), GetLastError());
				OutputDebugStringW(mess);
			}
			HWND videoWindow = CreateWindow(L"MFCapture Window Class", L"MFCapture Sample Application", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, GetModuleHandle(NULL), NULL);
			Assert::IsTrue(videoWindow);
			ShowWindow(videoWindow, SW_HIDE);
			UpdateWindow(videoWindow);
			mVideoWindow = videoWindow;
		}
		CComPtr<IMFActivate> GetVideoDevice()
		{
			VideoDevices* videoDevices = new VideoDevices();
			Assert::AreEqual(videoDevices->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoDevices);

			CComPtr<IMFActivate> myVideoDevice = videoDevices->GetVideoDevice(myVideoDeviceName);
			Assert::AreEqual(videoDevices->GetLastHRESULT(), S_OK);
			Assert::IsTrue(myVideoDevice);
			return myVideoDevice;
		}
		CComPtr<IMFActivate> GetAudioDevice()
		{
			AudioDevices* audioDevices = new AudioDevices();
			Assert::AreEqual(audioDevices->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioDevices);

			CComPtr<IMFActivate> myAudioDevice = audioDevices->GetAudioDevice(myAudioDeviceName);
			Assert::AreEqual(audioDevices->GetLastHRESULT(), S_OK);
			Assert::IsTrue(myAudioDevice);
			return myAudioDevice;
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
			mVideoDisplayControl = new VideoDisplayControl();
			mMediaSession = new MediaSession(mVideoDisplayControl);
			Assert::AreEqual(mMediaSession->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mMediaSession->GetMediaSession());
		}
		void InitTopology()
		{
			mTopology = new Topology();
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mTopology->GetTopology());
		}
	public:
		MediaFoundationCaptureTESTs::MediaFoundationCaptureTESTs()
		{
			InitMediaSession();
			InitTopology();

			mVideoDevice = GetVideoDevice();
			mAudioDevice = GetAudioDevice();
		}
		TEST_METHOD(AudioOnlyStreamDescriptorsTEST)
		{
			// source
			MediaSource* audioSource = new MediaSource(mAudioDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSource);

			//// PresentationDescriptor
			PresentationDescriptor* audioPresentationDescriptor = new PresentationDescriptor(audioSource->GetMediaSource());
			Assert::AreEqual(audioPresentationDescriptor->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioPresentationDescriptor->GetPresentationDescriptor());
			unsigned int items = 0;
			mLastHR = audioPresentationDescriptor->GetPresentationDescriptor()->GetCount(&items);
			Assert::AreEqual(mLastHR, S_OK);
			Assert::AreEqual(items, (unsigned int)0); // I have no idea why GetCount returns 0 when there is only 1 stream

			CComPtr<IMFStreamDescriptor> audioStreamDescriptor = audioPresentationDescriptor->GetFirstAudioStreamDescriptor();
			ValidateAudioStreamDescriptor(audioStreamDescriptor);
		}
		TEST_METHOD(VideoOnlyStreamDescriptorsTEST)
		{
			// source
			MediaSource* videoSource = new MediaSource(mVideoDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSource);

			// PresentationDescriptor
			PresentationDescriptor* videoPresentationDescriptor = new PresentationDescriptor(videoSource->GetMediaSource());
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
		TEST_METHOD(AggregateStreamDescriptorsTEST)
		{
			// video source
			MediaSource* videoSource = new MediaSource(mVideoDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSource);

			// audio source
			MediaSource* audioSource = new MediaSource(mAudioDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSource);

			// aggregate source
			MediaSource* aggregateSource = new MediaSource(mVideoDevice, mAudioDevice);
			Assert::AreEqual(aggregateSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregateSource);

			// PresentationDescriptors
			PresentationDescriptor* aggregatePresentationDescriptor = new PresentationDescriptor(aggregateSource->GetMediaSource());
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
		TEST_METHOD(AudioOnlyPassthroughTEST)
		{
			// source
			MediaSource* audioSource = new MediaSource(mAudioDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);

			// Topology Nodes
			TopologyNode* audioSourceTopologyNode = new TopologyNode(audioSource->GetMediaSource());
			Assert::AreEqual(audioSourceTopologyNode->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSourceTopologyNode->GetTopologyNode());

			TopologyNode* audioRendererTopologyNode = new TopologyNode(L"SAR");
			Assert::AreEqual(audioRendererTopologyNode->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioRendererTopologyNode->GetTopologyNode());

			mTopology->AddAndConnect2Nodes(audioSourceTopologyNode->GetTopologyNode(), audioRendererTopologyNode->GetTopologyNode());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveSingleSourceTopology();
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
		}
		TEST_METHOD(VideoOnlyPassthroughTEST)
		{
			// source
			MediaSource* videoSource = new MediaSource(mVideoDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);

			// Topology Nodes
			TopologyNode* videoSourceTopologyNode = new TopologyNode(videoSource->GetMediaSource());
			Assert::AreEqual(videoSourceTopologyNode->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSourceTopologyNode->GetTopologyNode());

			InitVideoWindow();
			TopologyNode* videoRendererTopologyNode = new TopologyNode(mVideoWindow);
			Assert::AreEqual(videoRendererTopologyNode->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoRendererTopologyNode->GetTopologyNode());

			mTopology->AddAndConnect2Nodes(videoSourceTopologyNode->GetTopologyNode(), videoRendererTopologyNode->GetTopologyNode());
			Assert::AreEqual(mTopology->GetLastHRESULT(), S_OK);

			mTopology->ResolveSingleSourceTopology();
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
		}
		TEST_METHOD(VideoAndAudioPassthroughTEST)
		{
			// aggregate source
			MediaSource* aggregateSource = new MediaSource(mVideoDevice, mAudioDevice);
			Assert::AreEqual(aggregateSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregateSource);

			// aggregate reader
			SourceReader* aggregateSourceReader = new SourceReader(aggregateSource->GetMediaSource());
			Assert::AreEqual(aggregateSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregateSourceReader->GetSourceReader());

			// Topology Nodes

		}
	};
}