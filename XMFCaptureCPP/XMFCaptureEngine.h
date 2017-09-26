#include <memory>
#include <atlcomcli.h>

class XMFCaptureDevice;
class XMFCaptureEngineRep;
struct IMFActivate;
class XMFCaptureEngine
{
public:
	XMFCaptureEngine(HWND hwnd, std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice);
	virtual ~XMFCaptureEngine();

	HRESULT StartPreview();
	HRESULT StopPreview();

	HRESULT StartRecord(PCWSTR pszDestinationFile);
	HRESULT StopRecord();

	void SleepState(bool fSleeping);

	bool IsPreviewing() const;
	bool IsRecording() const;
	HRESULT get_FramesCaptured(unsigned long* pVal) const;
	HRESULT	get_FPSForCapture(long* pVal) const;
	UINT ErrorID() const;

private:
	XMFCaptureEngine();

	XMFCaptureEngineRep* m_RepPtr;
};

//LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch (uMsg)
//	{
//		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
//		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
//		HANDLE_MSG(hwnd, WM_SIZE, OnSize);
//		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
//		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
//
//	case WM_ERASEBKGND:
//		return 1;
//
//	case WM_APP_CAPTURE_EVENT:
//	{
//		if (g_pEngine)
//		{
//			HRESULT hr = g_pEngine->OnCaptureEvent(wParam, lParam);
//			if (FAILED(hr))
//			{
//				ShowError(hwnd, g_pEngine->ErrorID(), hr);
//				InvalidateRect(hwnd, NULL, FALSE);
//			}
//		}
//
//		UpdateUI(hwnd);
//	}
//	return 0;
//	case WM_POWERBROADCAST:
//	{
//		switch (wParam)
//		{
//		case PBT_APMSUSPEND:
//			DbgPrint(L"++WM_POWERBROADCAST++ Stopping both preview & record stream.\n");
//			g_fSleepState = true;
//			g_pEngine->SleepState(g_fSleepState);
//			g_pEngine->StopRecord();
//			g_pEngine->StopPreview();
//			g_pEngine->DestroyCaptureEngine();
//			DbgPrint(L"++WM_POWERBROADCAST++ streams stopped, capture engine destroyed.\n");
//			break;
//		case PBT_APMRESUMEAUTOMATIC:
//			DbgPrint(L"++WM_POWERBROADCAST++ Reinitializing capture engine.\n");
//			g_fSleepState = false;
//			g_pEngine->SleepState(g_fSleepState);
//			g_pEngine->InitializeCaptureManager(hPreview, pSelectedDevice);
//			break;
//		case PBT_POWERSETTINGCHANGE:
//		{
//			// We should only be in here for GUID_MONITOR_POWER_ON.
//			POWERBROADCAST_SETTING* pSettings = (POWERBROADCAST_SETTING*)lParam;
//
//			// If this is a SOC system (AoAc is true), we want to check our current
//			// sleep state and based on whether the monitor is being turned on/off,
//			// we can turn off our media streams and/or re-initialize the capture
//			// engine.
//			if (pSettings != NULL && g_pwrCaps.AoAc && pSettings->PowerSetting == GUID_MONITOR_POWER_ON)
//			{
//				DWORD   dwData = *((DWORD*)pSettings->Data);
//				if (dwData == 0 && !g_fSleepState)
//				{
//					// This is a AOAC machine, and we're about to turn off our monitor, let's stop recording/preview.
//					DbgPrint(L"++WM_POWERBROADCAST++ Stopping both preview & record stream.\n");
//					g_fSleepState = true;
//					g_pEngine->SleepState(g_fSleepState);
//					g_pEngine->StopRecord();
//					g_pEngine->StopPreview();
//					g_pEngine->DestroyCaptureEngine();
//					DbgPrint(L"++WM_POWERBROADCAST++ streams stopped, capture engine destroyed.\n");
//				}
//				else if (dwData != 0 && g_fSleepState)
//				{
//					DbgPrint(L"++WM_POWERBROADCAST++ Reinitializing capture engine.\n");
//					g_fSleepState = false;
//					g_pEngine->SleepState(g_fSleepState);
//					g_pEngine->InitializeCaptureManager(hPreview, pSelectedDevice);
//				}
//			}
//		}
//		break;
//		case PBT_APMRESUMESUSPEND:
//		default:
//			// Don't care about this one, we always get the resume automatic so just
//			// latch onto that one.
//			DbgPrint(L"++WM_POWERBROADCAST++ (wParam=%u,lParam=%u)\n", wParam, lParam);
//			break;
//		}
//	}
//	return 1;
//	}
//	return DefWindowProc(hwnd, uMsg, wParam, lParam);
//}