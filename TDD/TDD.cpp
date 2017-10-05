
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "MediaFoundationTDD.h"
#include "AttributesFactory.h"
#include "Devices.h"

#include <comutil.h>
#include <algorithm> 

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
			Assert::AreEqual((int)myMFTDD->GetTopology(), NULL);
			myMFTDD->CreateTopology();
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myMFTDD->GetTopology(), NULL);
		}
		TEST_METHOD(CreateTopologyTEST)
		{
			Assert::AreEqual((int)myMFTDD->GetTopology(), NULL);
			myMFTDD->CreateTopology();
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myMFTDD->GetTopology(), NULL);
		}

		TEST_METHOD(CreateVideoOnlyMediaSourceTEST)
		{
			// video
			IMFActivate* myVideoDevice = myMFTDD->CreateVideoOnlyDevice(myVideoDeviceName);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myVideoDevice, NULL);

			// audio
			IMFAttributes* myAudioDeviceAttributes = myAttributesFactory->CreateAudioDeviceAttributes();
			Assert::AreEqual(myAttributesFactory->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myAudioDeviceAttributes, NULL);

			Devices* myAudioDevices = new Devices(myAudioDeviceAttributes);
			Assert::AreEqual(myAudioDevices->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myAudioDevices->GetDevices(), NULL);
			Assert::IsTrue(myAudioDevices->GetNumDevices() > 0);

			std::vector<std::wstring> myAudioDeviceNames = myAudioDevices->GetDeviceNames();
			bool foundAudioDevice = false;
			if (std::find(myAudioDeviceNames.begin(), myAudioDeviceNames.end(), myAudioDeviceName) != myAudioDeviceNames.end())
			{
				foundAudioDevice = true;
			}
			Assert::IsTrue(foundAudioDevice);
			IMFActivate* myAudioDevice = myAudioDevices->GetDeviceByName(myAudioDeviceName);
			Assert::AreNotEqual((int)myVideoDevice, NULL);

			// setup
			Assert::AreEqual((int)myMFTDD->GetMediaSource(), NULL);
			myMFTDD->CreateMediaSource(myVideoDevice);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myMFTDD->GetMediaSource(), NULL);

			IMFAttributes* mySourceReaderAsycCallbackAttributes = myAttributesFactory->CreateSourceReaderAsycCallbackAttributes(NULL);
			Assert::AreEqual(myAttributesFactory->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)mySourceReaderAsycCallbackAttributes, NULL);

			Assert::AreEqual((int)myMFTDD->GetSourceReader(), NULL);
			myMFTDD->CreateSourceReader(myMFTDD->GetMediaSource(), mySourceReaderAsycCallbackAttributes);
			Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myMFTDD->GetSourceReader(), NULL);

			delete myAttributesFactory;
		}
	};
}