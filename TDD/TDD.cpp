
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "TopologyNode.h"
#include "PresentationDescriptor.h"
#include "CaptureMediaSession.h"
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
		std::unique_ptr<CaptureMediaSession>	mCaptureMediaSession = nullptr;
		std::unique_ptr<Topology>		mTopology = nullptr;
		std::shared_ptr<VideoDisplayControl> mVideoDisplayControl = nullptr;
		CComPtr<IMFActivate> mAudioCaptureDevice = nullptr;
		CComPtr<IMFActivate> mVideoCaptureDevice = nullptr;
		CComPtr<IMFActivate> mAudioRenderer = nullptr;
		CComPtr<IMFActivate> mVideoRenderer = nullptr;
		HWND mVideoWindow = nullptr;

		void InitializeWindow()
		{
			WNDCLASS wc = { 0 };
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandle(nullptr);
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wc.lpszClassName = CLASS_NAME;
			Assert::IsTrue(RegisterClass(&wc));

			mVideoWindow = CreateWindow(CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
			Assert::IsTrue(mVideoWindow);

			ShowWindow(mVideoWindow, SW_SHOWDEFAULT);
			UpdateWindow(mVideoWindow);
		}
		void StartAndStopFor(DWORD milliseconds)
		{
			Assert::AreEqual(mCaptureMediaSession->GetLastHRESULT(), S_OK);
			mCaptureMediaSession->Start();
			Assert::AreEqual(mCaptureMediaSession->GetLastHRESULT(), S_OK);

			::Sleep(milliseconds);

			mCaptureMediaSession->Stop();
			Assert::AreEqual(mCaptureMediaSession->GetLastHRESULT(), S_OK);
		}
		void InitMediaSession()
		{
			mCaptureMediaSession = std::make_unique<CaptureMediaSession>(myVideoDeviceName, myAudioDeviceName, myCaptureFilePath);
			Assert::AreEqual(mCaptureMediaSession->GetLastHRESULT(), S_OK);
		}
	public:
		MediaFoundationCaptureTESTs::MediaFoundationCaptureTESTs()
		{
			InitializeWindow();
			InitMediaSession();
		}
		TEST_METHOD(CaptureAndPassthroughStartStop)
		{
			mCaptureMediaSession->InitCaptureAndPassthrough(mVideoWindow, L"CaptureAndPassthrough.ts");
			StartAndStopFor(5000);
		}
	};
}