
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "MediaFoundationTDD.h"
#include "AttributesFactory.h"
#include "SourceReader.h"
#include "TopologyNode.h"
#include "PresentationDescriptor.h"

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
		AttributesFactory* myAttributesFactory;
		MediaFoundationTDD* myMFTDD = NULL;
		HRESULT mLastHR = S_OK;

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
			myAttributesFactory = new AttributesFactory();
			Assert::AreEqual(myAttributesFactory->GetLastHRESULT(), S_OK);

			myMFTDD = new MediaFoundationTDD();
			Assert::AreNotEqual(NULL, (int)myMFTDD);
		}
		TEST_METHOD(CreateMediaSessionTEST)
		{
			Assert::IsTrue(!myMFTDD->GetTopology());
			myMFTDD->CreateTopology();
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(myMFTDD->GetTopology());
		}
		TEST_METHOD(CreateEmptyTopologyTEST)
		{
			Assert::IsTrue(!myMFTDD->GetTopology());
			myMFTDD->CreateTopology();
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(myMFTDD->GetTopology());
		}
		TEST_METHOD(CreateAudioOnlySourceReaderTEST)
		{
			// audio device
			IMFActivate* myAudioDevice = myMFTDD->CreateAudioDevice(myAudioDeviceName);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myAudioDevice, NULL);

			// source
			CComPtr<IMFMediaSource>	audioSource = myMFTDD->CreateMediaSource(myAudioDevice);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSource);

			// reader
			SourceReader* audioSourceReader = new SourceReader(audioSource);
			Assert::AreEqual(audioSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSourceReader->GetSourceReader());
			Assert::IsTrue(audioSourceReader->GetMediaSource());

			// PresentationDescriptor
			PresentationDescriptor* audioPresentationDescriptor = new PresentationDescriptor(audioSource);
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
			IMFActivate* myVideoDevice = myMFTDD->CreateVideoDevice(myVideoDeviceName);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myVideoDevice, NULL);

			// source
			CComPtr<IMFMediaSource>	videoSource = myMFTDD->CreateMediaSource(myVideoDevice);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSource);

			// reader
			SourceReader* videoSourceReader = new SourceReader(videoSource);
			Assert::AreEqual(videoSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSourceReader->GetSourceReader());
			Assert::IsTrue(videoSourceReader->GetMediaSource());

			// PresentationDescriptor
			PresentationDescriptor* videoPresentationDescriptor = new PresentationDescriptor(videoSource);
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
			IMFActivate* myVideoDevice = myMFTDD->CreateVideoDevice(myVideoDeviceName);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myVideoDevice, NULL);

			// audio device
			IMFActivate* myAudioDevice = myMFTDD->CreateAudioDevice(myAudioDeviceName);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myAudioDevice, NULL);

			// video source
			CComPtr<IMFMediaSource>	videoSource = myMFTDD->CreateMediaSource(myVideoDevice);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSource);

			// audio source
			CComPtr<IMFMediaSource>	audioSource = myMFTDD->CreateMediaSource(myAudioDevice);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSource);

			// aggregate source
			CComPtr<IMFMediaSource>	aggregateSource = myMFTDD->CreateAggregateMediaSource(videoSource, audioSource);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregateSource);

			// reader
			SourceReader* aggregateSourceReader = new SourceReader(aggregateSource);
			Assert::AreEqual(aggregateSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregateSourceReader->GetSourceReader());
			Assert::IsTrue(aggregateSourceReader->GetMediaSource());

			// PresentationDescriptors
			PresentationDescriptor* aggregatePresentationDescriptor = new PresentationDescriptor(aggregateSource);
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
			TopologyNode* videoSourceTopologyNode = new TopologyNode(videoSource);
			Assert::AreEqual(videoSourceTopologyNode->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSourceTopologyNode->GetTopologyNode());

			TopologyNode* audioSourceTopologyNode = new TopologyNode(audioSource);
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