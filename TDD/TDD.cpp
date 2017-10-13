
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

#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfapi.h>

namespace MediaFoundationTesing
{		
	TEST_CLASS(MediaFoundationCaptureTESTs)
	{
	private:
		std::wstring myVideoDeviceName = L"XI100DUSB-SDI Video";							//<-------------------Video device to test-----------------------------
		std::wstring myAudioDeviceName = L"Digital Audio Interface (XI100DUSB-SDI Audio)";	//<-------------------Audio device to test-----------------------------
		HRESULT mLastHR = S_OK;

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
	public:
		MediaFoundationCaptureTESTs::MediaFoundationCaptureTESTs()
		{
		}
		TEST_METHOD(CreateMediaSessionTEST)
		{
			MediaSession* mediaSession = new MediaSession();
			Assert::AreEqual(mediaSession->GetLastHRESULT(), S_OK);
			Assert::IsTrue(mediaSession->GetMediaSession());
		}
		TEST_METHOD(CreateEmptyTopologyTEST)
		{
			Topology* myTopology = new Topology();
			Assert::AreEqual(myTopology->GetLastHRESULT(), S_OK);
			Assert::IsTrue(myTopology->GetTopology());
		}
		TEST_METHOD(CreateAudioOnlySourceReaderTEST)
		{
			// audio device
			CComPtr<IMFActivate> myAudioDevice = GetAudioDevice();
			Assert::IsTrue(myAudioDevice);

			// source
			MediaSource* audioSource = new MediaSource(myAudioDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSource);

			// reader
			SourceReader* audioSourceReader = new SourceReader(audioSource->GetMediaSource());
			Assert::AreEqual(audioSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSourceReader->GetSourceReader());

			// PresentationDescriptor
			PresentationDescriptor* audioPresentationDescriptor = new PresentationDescriptor(audioSource->GetMediaSource());
			Assert::AreEqual(audioSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioPresentationDescriptor->GetPresentationDescriptor());
			unsigned int items = 0;
			mLastHR = audioPresentationDescriptor->GetPresentationDescriptor()->GetCount(&items);
			Assert::AreEqual(mLastHR, S_OK);
			Assert::AreEqual(items, (unsigned int)0); // why not 1?
		}
		TEST_METHOD(CreateVideoOnlySourceReaderTEST)
		{
			// video device
			CComPtr<IMFActivate> myVideoDevice = GetVideoDevice();
			Assert::IsTrue(myVideoDevice);

			// source
			MediaSource* videoSource = new MediaSource(myVideoDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSource);

			// reader
			SourceReader* videoSourceReader = new SourceReader(videoSource->GetMediaSource());
			Assert::AreEqual(videoSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSourceReader->GetSourceReader());

			// PresentationDescriptor
			PresentationDescriptor* videoPresentationDescriptor = new PresentationDescriptor(videoSource->GetMediaSource());
			Assert::AreEqual(videoSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoPresentationDescriptor->GetPresentationDescriptor());
			unsigned int items = 0;
			mLastHR = videoPresentationDescriptor->GetPresentationDescriptor()->GetCount(&items);
			Assert::AreEqual(mLastHR, S_OK);
			Assert::AreEqual(items, (unsigned int)0); // why not 1?
		}
		TEST_METHOD(CreateVideoAndAudioSourceReaderTEST)
		{
			// video device
			CComPtr<IMFActivate> myVideoDevice = GetVideoDevice();
			Assert::IsTrue(myVideoDevice);

			// audio device
			CComPtr<IMFActivate> myAudioDevice = GetAudioDevice();
			Assert::IsTrue(myAudioDevice);

			// video source
			MediaSource* videoSource = new MediaSource(myVideoDevice);
			Assert::AreEqual(videoSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSource);

			// audio source
			MediaSource* audioSource = new MediaSource(myAudioDevice);
			Assert::AreEqual(audioSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSource);

			// aggregate source
			MediaSource* aggregateSource = new MediaSource(myVideoDevice, myAudioDevice);
			Assert::AreEqual(aggregateSource->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregateSource);


			// aggregate reader
			SourceReader* aggregateSourceReader = new SourceReader(aggregateSource->GetMediaSource());
			Assert::AreEqual(aggregateSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregateSourceReader->GetSourceReader());

			// PresentationDescriptors
			PresentationDescriptor* aggregatePresentationDescriptor = new PresentationDescriptor(aggregateSource->GetMediaSource());
			Assert::AreEqual(aggregateSourceReader->GetLastHRESULT(), S_OK);
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

			// Topology Nodes
			TopologyNode* videoSourceTopologyNode = new TopologyNode(videoSource->GetMediaSource());
			Assert::AreEqual(videoSourceTopologyNode->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSourceTopologyNode->GetTopologyNode());

			TopologyNode* audioSourceTopologyNode = new TopologyNode(audioSource->GetMediaSource());
			Assert::AreEqual(audioSourceTopologyNode->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSourceTopologyNode->GetTopologyNode());

			TopologyNode* videoRendererTopologyNode = new TopologyNode(NULL);
			Assert::AreEqual(videoRendererTopologyNode->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoRendererTopologyNode->GetTopologyNode());

			TopologyNode* audioRendererTopologyNode = new TopologyNode(L"SAR");
			Assert::AreEqual(audioRendererTopologyNode->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioRendererTopologyNode->GetTopologyNode());
		}
	};
}