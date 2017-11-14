#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "CaptureMediaSession.h"

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
		static std::wstring	myVideoDeviceName;
		static std::wstring	myAudioDeviceName;
		static std::wstring	myCaptureFilePath;
		static std::unique_ptr<CaptureMediaSession>	mCaptureMediaSession;
		static HWND mVideoWindow;
		static bool mVideoWindowClassIsRegistered;

		HRESULT mLastHR = S_OK;

		static void InitializeWindow()
		{
			if (!mVideoWindowClassIsRegistered)
			{
				WNDCLASS wc = { 0 };
				wc.lpfnWndProc = WindowProc;
				wc.hInstance = GetModuleHandle(nullptr);
				wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
				wc.lpszClassName = CLASS_NAME;
				Assert::IsTrue(RegisterClass(&wc));
				mVideoWindowClassIsRegistered = true;
			}

			mVideoWindow = CreateWindow(CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
			Assert::IsTrue(mVideoWindow);

			ShowWindow(mVideoWindow, SW_SHOWDEFAULT);
			UpdateWindow(mVideoWindow);
		}
		static void StartAndStopFor(DWORD milliseconds)
		{
			Assert::AreEqual(mCaptureMediaSession->GetLastHRESULT(), S_OK);
			mCaptureMediaSession->Start();
			Assert::AreEqual(mCaptureMediaSession->GetLastHRESULT(), S_OK);

			::Sleep(milliseconds);

			mCaptureMediaSession->Stop();
			Assert::AreEqual(mCaptureMediaSession->GetLastHRESULT(), S_OK);
		}
		static void InitMediaSession()
		{
			mCaptureMediaSession = std::make_unique<CaptureMediaSession>(myVideoDeviceName, myAudioDeviceName, myCaptureFilePath);
			Assert::AreEqual(mCaptureMediaSession->GetLastHRESULT(), S_OK);
		}
	public:
		MediaFoundationCaptureTESTs::MediaFoundationCaptureTESTs()
		{
			if (mCaptureMediaSession == nullptr)
			{
				InitMediaSession();
			}
			if (mVideoWindow == nullptr)
			{
				InitializeWindow();
			}
		}
		MediaFoundationCaptureTESTs::~MediaFoundationCaptureTESTs()
		{
			if (mVideoWindow)
			{
				BOOL res = DestroyWindow(mVideoWindow);
				if (res == FALSE)
				{
					OutputDebugStringW(L"Failed to DestroyWindow!");
				}
				mVideoWindow = nullptr;
			}
		}

		TEST_METHOD(CaptureAndPassthroughStartStop)
		{
			mCaptureMediaSession->InitCaptureAndPassthrough(mVideoWindow, L"CaptureAndPassthrough.ts");
			StartAndStopFor(1000);
		}
		TEST_METHOD(PassthroughStartStop)
		{
			mCaptureMediaSession->InitPassthrough(mVideoWindow);
			StartAndStopFor(1000);
		}
	};
	std::wstring MediaFoundationCaptureTESTs::myVideoDeviceName = L"XI100DUSB-SDI Video";		//<-------------------Video device to test-----------------------------
	std::wstring MediaFoundationCaptureTESTs::myAudioDeviceName = L"XI100DUSB-SDI Audio";		//<-------------------Audio device to test-----------------------------
	//std::wstring MediaFoundationCaptureTESTs::myVideoDeviceName = L"XI100DUSB-HDMI Video";	//<-------------------Video device to test-----------------------------
	//std::wstring MediaFoundationCaptureTESTs::myAudioDeviceName = L"XI100DUSB-HDMI Audio";	//<-------------------Audio device to test-----------------------------
	std::wstring MediaFoundationCaptureTESTs::myCaptureFilePath = L"C:\\";						//<-------------------Path to file captures----------------------------
	std::unique_ptr<CaptureMediaSession> MediaFoundationCaptureTESTs::mCaptureMediaSession = nullptr;
	HWND MediaFoundationCaptureTESTs::mVideoWindow = nullptr;
	bool MediaFoundationCaptureTESTs::mVideoWindowClassIsRegistered = false;
}