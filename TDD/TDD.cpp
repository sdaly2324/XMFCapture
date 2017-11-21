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
		HRESULT mLastHR = S_OK;

		static void InitializeWindow()
		{
			WNDCLASS wc = { 0 };
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandle(nullptr);
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wc.lpszClassName = CLASS_NAME;
			ATOM atom = RegisterClass(&wc);
			if (atom == 0)
			{
				DWORD err = GetLastError();
				LPVOID lpMsgBuf;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);
				WCHAR mess[1024];
				swprintf_s(mess, 1024, L"InitializeWindow RegisterClass FAILED - %s\n", (LPTSTR)lpMsgBuf);
				OutputDebugStringW(mess);
				LocalFree(lpMsgBuf);
			}
			else
			{
				OutputDebugStringW(L"InitializeWindow RegisterClass SUCCEEDED\n");
			}

			mVideoWindow = CreateWindow(CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
			Assert::IsTrue(mVideoWindow);

			ShowWindow(mVideoWindow, SW_SHOWDEFAULT);
			UpdateWindow(mVideoWindow);
		}
		static void InitMediaSession()
		{
			mCaptureMediaSession = std::make_unique<CaptureMediaSession>();
			Assert::AreEqual(mCaptureMediaSession->InitializeCapturer(), S_OK);
			Assert::AreEqual(mCaptureMediaSession->SetVideoWindow((LONG)mVideoWindow, 0, 0, 640, 480 ), S_OK);
		}
		static void LogTime(std::wstring label, unsigned int durationInSeconds)
		{
			WCHAR mess[1024];
			long long prevTime = 0;
			for (unsigned int x = 0; x < durationInSeconds; x++)
			{
				Sleep(1000);
				unsigned long framesCaptured = 0;
				long long currentTime = mCaptureMediaSession->get_FramesCaptured(&framesCaptured);
				double deltaTimeInMiliSeconds = (((double)currentTime - (double)prevTime) / 10000);
				swprintf_s(mess, 1024, L"POOP %s framesCaptured(%d)\n", label.c_str(), framesCaptured);
				OutputDebugStringW(mess);
				prevTime = currentTime;
			}
		}
	public:
		MediaFoundationCaptureTESTs::MediaFoundationCaptureTESTs()
		{
			if (mVideoWindow == nullptr)
			{
				InitializeWindow();
			}
			if (mCaptureMediaSession == nullptr)
			{
				InitMediaSession();
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
				res = UnregisterClass(CLASS_NAME, nullptr);
				if (res == FALSE)
				{
					OutputDebugStringW(L"Failed to DestroyWindow!");
				}
			}
			if (mCaptureMediaSession)
			{
				mCaptureMediaSession.release();
			}
		}

		TEST_METHOD(Capture)
		{
			Assert::AreEqual(mCaptureMediaSession->StartCapture(_I64_MAX, L"C:\\Capture"), S_OK);

			LogTime(L"Capture", 2);

			Assert::AreEqual(mCaptureMediaSession->StopCapture(), S_OK);
		}
		TEST_METHOD(Preview)
		{
			Assert::AreEqual(mCaptureMediaSession->SetPreviewOutputOn(true), S_OK);

			LogTime(L"Preview", 2);

			Assert::AreEqual(mCaptureMediaSession->SetPreviewOutputOn(false), S_OK);
		}
		TEST_METHOD(TogglePreview)
		{
			Assert::AreEqual(mCaptureMediaSession->StartCapture(_I64_MAX, L"C:\\TogglePreview"), S_OK);

			LogTime(L"TogglePreview StartCapture", 2);

			Assert::AreEqual(mCaptureMediaSession->SetPreviewOutputOn(false), S_OK);
			ShowWindow(mVideoWindow, SW_HIDE);

			LogTime(L"TogglePreview StopPreview", 2);

			Assert::AreEqual(mCaptureMediaSession->SetPreviewOutputOn(true), S_OK);
			ShowWindow(mVideoWindow, SW_SHOWDEFAULT);

			LogTime(L"TogglePreview StartPreview", 2);

			Assert::AreEqual(mCaptureMediaSession->StopCapture(), S_OK);
		}
		TEST_METHOD(Pause)
		{
			Assert::AreEqual(mCaptureMediaSession->StartCapture(_I64_MAX, L"C:\\Pause"), S_OK);

			LogTime(L"Pause StartCapture", 2);

			Assert::AreEqual(mCaptureMediaSession->PauseCapture(false), S_OK);

			LogTime(L"Pause PauseCapture", 2);

			Assert::AreEqual(mCaptureMediaSession->ResumeCapture(false), S_OK);

			LogTime(L"Pause ResumeCapture", 3);

			Assert::AreEqual(mCaptureMediaSession->StopCapture(), S_OK);
		}
	};
	std::unique_ptr<CaptureMediaSession> MediaFoundationCaptureTESTs::mCaptureMediaSession = nullptr;
	HWND MediaFoundationCaptureTESTs::mVideoWindow = nullptr;
}