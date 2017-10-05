
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "MediaFoundationTDD.h"
#include "AttributesFactory.h"
#include "Devices.h"

#include <comutil.h>
#include <algorithm> 

namespace MediaFoundationTesing
{		
	TEST_CLASS(MediaFoundationTESTs)
	{
	private:
		AttributesFactory* myAttributesFactory;
		MediaFoundationTDD* myMFTDD = NULL;
		std::wstring myDeviceName = L"XI100DUSB-SDI Video";
	public:
		MediaFoundationTESTs::MediaFoundationTESTs()
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
			IMFAttributes* myVideoDeviceAttributes = myAttributesFactory->CreateVideoDeviceAttributes();
			Assert::AreEqual(myAttributesFactory->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myVideoDeviceAttributes, NULL);

			Devices* myDevices = new Devices(myVideoDeviceAttributes);
			Assert::AreEqual(myDevices->GetLastHRESULT(), S_OK);
			Assert::AreNotEqual((int)myDevices->GetDevices(), NULL);
			Assert::IsTrue(myDevices->GetNumDevices() > 0);

			std::vector<std::wstring> myDeviceNames = myDevices->GetDeviceNames();
			bool found_XI100DUSB_SDI_Video = false;
			if (std::find(myDeviceNames.begin(), myDeviceNames.end(), myDeviceName) != myDeviceNames.end())
			{
				found_XI100DUSB_SDI_Video = true;
			}
			Assert::IsTrue(found_XI100DUSB_SDI_Video);
			IMFActivate* myDevice = myDevices->GetDeviceByName(myDeviceName);
			Assert::AreNotEqual((int)myDevice, NULL);

			Assert::AreEqual((int)myMFTDD->GetMediaSource(), NULL);
			myMFTDD->CreateMediaSource(myDevice);
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
			delete myDevices;
		}
	};
}