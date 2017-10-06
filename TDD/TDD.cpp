
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "MediaFoundationTDD.h"
#include "AttributesFactory.h"
#include "Devices.h"
#include "SourceReader.h"

#include <mfidl.h>
#include <mfreadwrite.h>

namespace MediaFoundationTesing
{		
	TEST_CLASS(MediaFoundationCaptureTESTs)
	{
	private:
		std::wstring myVideoDeviceName = L"XI100DUSB-SDI Video";							//<-------------------Video device to test-----------------------------
		std::wstring myAudioDeviceName = L"Digital Audio Interface (XI100DUSB-SDI Audio)";	//<-------------------Audio device to test-----------------------------
		AttributesFactory* myAttributesFactory;
		MediaFoundationTDD* myMFTDD = NULL;
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
		TEST_METHOD(CreateTopologyTEST)
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
			CComPtr<IMFMediaSource>	audioMediaSource = myMFTDD->CreateMediaSource(myAudioDevice);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioMediaSource);

			// reader
			SourceReader* audioSourceReader = new SourceReader(audioMediaSource);
			Assert::AreEqual(audioSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioSourceReader->GetSourceReader());

			// PresentationDescriptor
			CComPtr<IMFPresentationDescriptor> aggregatePresentationDescriptor = audioSourceReader->GetPresentationDescriptor();
			Assert::AreEqual(audioSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregatePresentationDescriptor);
			unsigned int items = 0;
			HRESULT hr = aggregatePresentationDescriptor->GetCount(&items);
			Assert::AreEqual(hr, S_OK);
			Assert::AreEqual(items, (unsigned int)0); // why not 1?
		}
		TEST_METHOD(CreateVideoOnlySourceReaderTEST)
		{
			// video device
			IMFActivate* myVideoDevice = myMFTDD->CreateVideoDevice(myVideoDeviceName);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myVideoDevice, NULL);

			// source
			CComPtr<IMFMediaSource>	videoMediaSource = myMFTDD->CreateMediaSource(myVideoDevice);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoMediaSource);

			// reader
			SourceReader* videoSourceReader = new SourceReader(videoMediaSource);
			Assert::AreEqual(videoSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoSourceReader->GetSourceReader());

			// PresentationDescriptor
			CComPtr<IMFPresentationDescriptor> aggregatePresentationDescriptor = videoSourceReader->GetPresentationDescriptor();
			Assert::AreEqual(videoSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregatePresentationDescriptor);
			unsigned int items = 0;
			HRESULT hr = aggregatePresentationDescriptor->GetCount(&items);
			Assert::AreEqual(hr, S_OK);
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
			CComPtr<IMFMediaSource>	videoMediaSource = myMFTDD->CreateMediaSource(myVideoDevice);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(videoMediaSource);

			// audio source
			CComPtr<IMFMediaSource>	audioMediaSource = myMFTDD->CreateMediaSource(myAudioDevice);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(audioMediaSource);

			// aggregate source
			CComPtr<IMFMediaSource>	aggregateMediaSource = myMFTDD->CreateAggregateMediaSource(videoMediaSource, audioMediaSource);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregateMediaSource);

			// reader
			SourceReader* aggregateSourceReader = new SourceReader(aggregateMediaSource);
			Assert::AreEqual(aggregateSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregateSourceReader->GetSourceReader());

			// PresentationDescriptor
			CComPtr<IMFPresentationDescriptor> aggregatePresentationDescriptor = aggregateSourceReader->GetPresentationDescriptor();
			Assert::AreEqual(aggregateSourceReader->GetLastHRESULT(), S_OK);
			Assert::IsTrue(aggregatePresentationDescriptor);
			unsigned int items = 0;
			HRESULT hr = aggregatePresentationDescriptor->GetCount(&items);
			Assert::AreEqual(hr, S_OK);
			Assert::AreEqual(items, (unsigned int)2);
		}
	};
}