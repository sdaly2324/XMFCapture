
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "MediaFoundationTDD.h"
#include "Attributes.h"
#include "Devices.h"

#include <comutil.h>
#include <algorithm> 

namespace MediaFoundation
{		
	TEST_CLASS(MediaFoundation)
	{
	private:
		MediaFoundationTDD* myMFTDD = NULL;
		std::wstring myDeviceName = L"XI100DUSB-SDI Video";
	public:
		MediaFoundation::MediaFoundation()
		{
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
			Attributes* myAttributes = new Attributes();
			Assert::AreEqual((int)myAttributes->GetAttributes(), NULL);
			myAttributes->CreateVideoDeviceAttributes();
			Assert::AreNotEqual((int)myAttributes->GetAttributes(), NULL);

			Devices* myDevices = new Devices(myAttributes->GetAttributes());
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

			delete myAttributes;
			delete myDevices;
		}
		//TEST_METHOD(CreateSourceReaderTEST)
		//{
		//	Assert::AreEqual((int)myMFTDD->GetSourceReader(), NULL);
		//	myMFTDD->CreateSourceReader();
		//	Assert::AreEqual(myMFTDD->GetLastHRESULT(), S_OK);
		//	Assert::AreNotEqual((int)myMFTDD->GetSourceReader(), NULL);
		//}
	};
}