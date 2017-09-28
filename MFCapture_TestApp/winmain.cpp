#include "resource.h"
#include "XMFCaptureAPI.h"

#include <windows.h>
#include <windowsx.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <assert.h>
#include <shlwapi.h>
#include <Dbt.h>
#include <ks.h>
#include <ksmedia.h>
#include <atlstr.h>

// Include the v6 common controls in the manifest
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

XOSStringList	gDevices;
XMFCaptureAPI*	g_CaptureAPI = XMFCaptureAPI::GetInstance();

HDEVNOTIFY		g_hdevnotify = NULL;
HWND			g_hwnd = NULL;

bool IMFCaptureEngine = false;
bool IMFCaptureEnginePreview = false;
bool IMFSinkWriter = false;

const UINT32 TARGET_BIT_RATE = 240 * 1000;

const WCHAR CLASS_NAME [] = L"MFCapture Window Class";
const WCHAR WINDOW_NAME [] = L"MFCapture Sample Application";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

bool    InitializeWindow(HWND *pHwnd);

void    OnInitDialog(HWND hDlg);
void    OnCloseDialog();

void    UpdateUI(HWND hDlg);

void    StartCapture(HWND hDlg, bool useOld);
void    StopCapture(HWND hDlg);

void    StartPreview(HWND hDlg, HWND hwnd);
void    StopPreview(HWND hDlg);

HRESULT SetSelectedDevice(HWND hDlg);
HRESULT UpdateDeviceList(HWND hDlg);
void    OnDeviceChange(HWND hwnd, WPARAM reason, DEV_BROADCAST_HDR *pHdr);

void    NotifyError(HWND hwnd, const WCHAR *sErrorMessage, HRESULT hrErr);
void    EnableDialogControl(HWND hDlg, int nIDDlgItem, bool bEnable);

const UINT WM_APP_PREVIEW_ERROR = WM_APP + 1;

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, INT /*nCmdShow*/)
{
	(void) HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (InitializeWindow(&g_hwnd))
	{
		INT_PTR ret = DialogBox(
			hInstance,
			MAKEINTRESOURCE(IDD_DIALOG1),
			NULL,
			DialogProc
			);

		if (ret == 0 || ret == -1)
		{
			MessageBox(NULL, L"Could not create dialog", L"Error", MB_OK | MB_ICONERROR);
		}
	}
	return 0;
}

bool InitializeWindow(HWND *pHwnd)
{
	WNDCLASS wc = { 0 };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = CLASS_NAME;
	//wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);

	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	HWND hwnd = CreateWindow(
		CLASS_NAME,
		WINDOW_NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		640,
		480,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
		);

	if (!hwnd)
	{
		return FALSE;
	}

	ShowWindow(g_hwnd, SW_HIDE);
	UpdateWindow(g_hwnd);

	*pHwnd = hwnd;

	return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		//HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		//HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
		//HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		//HANDLE_MSG(hwnd, WM_SIZE, OnSize);

	case WM_APP_PREVIEW_ERROR:
		//ShowErrorMessage(L"Error", (HRESULT) wParam);
		break;

	case WM_DEVICECHANGE:
		//OnDeviceChange(hwnd, (PDEV_BROADCAST_HDR) lParam);
		break;

	case WM_ERASEBKGND:
		return 1;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		OnInitDialog(hDlg);
		break;

	case WM_DEVICECHANGE:
		OnDeviceChange(hDlg, wParam, (PDEV_BROADCAST_HDR) lParam);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CAPTURE_USING_IMFCaptureEngine:
			if (g_CaptureAPI->IsCapturing())
			{
				StopCapture(hDlg);
				IMFCaptureEngine = false;
			}
			else
			{
				IMFCaptureEngine = true;
				StartCapture(hDlg, false);
				//ShowWindow(g_hwnd, SW_SHOW);
			}
			UpdateUI(hDlg);
			return TRUE;
		case IDC_CAPTURE_USING_IMFSinkWriter:
			if (g_CaptureAPI->IsCapturing())
			{
				StopCapture(hDlg);
				IMFSinkWriter = false;
			}
			else
			{
				IMFSinkWriter = true;
				StartCapture(hDlg, true);
				//ShowWindow(g_hwnd, SW_SHOW);
			}
			UpdateUI(hDlg);
			return TRUE;
		case IDC_VIEW:
			if (IsWindowVisible(g_hwnd))
			{
				ShowWindow(g_hwnd, SW_HIDE);
				StopPreview(hDlg);
				IMFCaptureEnginePreview = false;
			}
			else
			{
				StartPreview(hDlg, g_hwnd);
				ShowWindow(g_hwnd, SW_SHOW);
				IMFCaptureEnginePreview = true;
			}
			UpdateWindow(g_hwnd);
			UpdateUI(hDlg);
			return TRUE;

		case IDCANCEL:
			OnCloseDialog();
			::EndDialog(hDlg, IDCANCEL);
			return TRUE;

		}
		break;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// OnInitDialog
// Handler for WM_INITDIALOG message.
//-----------------------------------------------------------------------------

void OnInitDialog(HWND hDlg)
{
	HRESULT hr = S_OK;

	// Register for device notifications
	if (SUCCEEDED(hr))
	{
		DEV_BROADCAST_DEVICEINTERFACE di = { 0 };

		di.dbcc_size = sizeof(di);
		di.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
		di.dbcc_classguid = KSCATEGORY_CAPTURE;

		g_hdevnotify = RegisterDeviceNotification(
			hDlg,
			&di,
			DEVICE_NOTIFY_WINDOW_HANDLE
			);

		if (g_hdevnotify == NULL)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = UpdateDeviceList(hDlg);
	}
	
	if (SUCCEEDED(hr))
	{
		UpdateUI(hDlg);
		if (!(gDevices.size() > 0))
		{
			::MessageBox(hDlg, TEXT("Could not find any capture devices."), TEXT("MFCaptureToFile"), MB_OK);
		}

	}
	else
	{
		OnCloseDialog();
		::EndDialog(hDlg, 0);
	}
}



//-----------------------------------------------------------------------------
// OnCloseDialog
// 
// Frees resources before closing the dialog.
//-----------------------------------------------------------------------------

void OnCloseDialog()
{
	if (g_CaptureAPI->IsCapturing())
	{
		g_CaptureAPI->StopCapture();
	}

	if (g_hdevnotify)
	{
		UnregisterDeviceNotification(g_hdevnotify);
	}

	CoUninitialize();
}


void StartCapture(HWND hDlg, bool useOld)
{
	HRESULT hr = S_OK;

	// Create the media source for the capture device.

	if (SUCCEEDED(hr))
	{
		hr = SetSelectedDevice(hDlg);
	}

	if (SUCCEEDED(hr))
	{
		std::shared_ptr<std::wstring> outputPath(new std::wstring(L"C:\\capture.ts"));
		g_CaptureAPI->SetOutputPath(outputPath);
		hr = g_CaptureAPI->StartCapture(useOld);
	}

	if (FAILED(hr))
	{
		NotifyError(hDlg, L"Error starting capture.", hr);
	}
}

void StopCapture(HWND hDlg)
{
	HRESULT hr = S_OK;

	hr = g_CaptureAPI->StopCapture();

	UpdateDeviceList(hDlg);

	// NOTE: Updating the device list releases the existing IMFActivate 
	// pointers. This ensures that the current instance of the video capture 
	// source is released.

	if (FAILED(hr))
	{
		NotifyError(hDlg, L"Error stopping capture. File might be corrupt.", hr);
	}
}

void StartPreview(HWND hDlg, HWND hwnd)
{
	HRESULT hr = S_OK;

	// Create the media source for the capture device.

	if (SUCCEEDED(hr))
	{
		hr = SetSelectedDevice(hDlg);
	}

	if (SUCCEEDED(hr))
	{
		hr = g_CaptureAPI->StartPreview(hwnd);
	}

	if (FAILED(hr))
	{
		NotifyError(hDlg, L"Error starting preview.", hr);
	}
}

void StopPreview(HWND hDlg)
{
	HRESULT hr = S_OK;

	hr = g_CaptureAPI->StopPreview();

	if (FAILED(hr))
	{
		NotifyError(hDlg, L"Error stopping preview.", hr);
	}
}


//-----------------------------------------------------------------------------
// CreateSelectedDevice
// 
// Create a media source for the video capture device selected by the user.
//-----------------------------------------------------------------------------

HRESULT SetSelectedDevice(HWND hDlg)
{
	int nIDDlgItem = IDC_VIDEO_AND_AUDIO_DEVICE_LIST;
	HWND hDeviceList = GetDlgItem(hDlg, nIDDlgItem);

	// First get the index of the selected item in the combo boxes.
	int iListIndex = ComboBox_GetCurSel(hDeviceList);
	if (iListIndex == CB_ERR)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD strLen = ComboBox_GetLBTextLen(hDeviceList, iListIndex);
	CString menuName;
	LRESULT result = ComboBox_GetLBText(hDeviceList, iListIndex, menuName.GetBuffer(strLen));
	if (result == CB_ERR)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	XOSString temp(new std::wstring(menuName));
	g_CaptureAPI->SetCurrentDevice(temp);

	return S_OK;
}

HRESULT InitPopupMenu(XOSStringList deviceNames, HWND hCombobox)
{
	HRESULT hr = S_OK;


	ComboBox_ResetContent(hCombobox);
	int index = 0;
	for (auto& element : deviceNames)
	{
		int iListIndex = ComboBox_AddString(hCombobox, element->c_str());
		if (iListIndex == CB_ERR || iListIndex == CB_ERRSPACE)
		{
			hr = E_FAIL;
		}
		index++;
	}

	return hr;
}


//-----------------------------------------------------------------------------
// UpdateDeviceList
// 
// Enumerates the video capture devices and populates the list of device
// names in the dialog UI.
//-----------------------------------------------------------------------------
HRESULT UpdateDeviceList(HWND hDlg)
{
	HRESULT hr = S_OK;

	hr = g_CaptureAPI->ReEnumerateDevices();
	if (SUCCEEDED(hr))
	{
		HWND hDeviceCombobox = GetDlgItem(hDlg, IDC_VIDEO_AND_AUDIO_DEVICE_LIST);
		int selection = ComboBox_GetCurSel(hDeviceCombobox);
		if (selection == -1)
		{
			selection = 0;
		}
		gDevices = g_CaptureAPI->GetDevicePairNamesList();
		hr = InitPopupMenu(gDevices, hDeviceCombobox);
		if (SUCCEEDED(hr) && gDevices.size() > 0)
		{
			ComboBox_SetCurSel(hDeviceCombobox, selection);
		}
	}

	return hr;
}

//-----------------------------------------------------------------------------
// UpdateUI
// 
// Updates the dialog UI for the current state.
//-----------------------------------------------------------------------------
void UpdateUI(HWND hDlg)
{
	bool bEnable = gDevices.size() > 0;     // Are there any capture devices?
	bool bCapturing = g_CaptureAPI->IsCapturing();     // Is video capture in progress now?

	HWND hButton = GetDlgItem(hDlg, IDC_CAPTURE_USING_IMFCaptureEngine);
	if (bCapturing && IMFCaptureEngine == true)
	{
		SetWindowText(hButton, L"Stop IMFCaptureEngine");
	}
	else
	{
		SetWindowText(hButton, L"Use IMFCaptureEngine");
	}

	hButton = GetDlgItem(hDlg, IDC_CAPTURE_USING_IMFSinkWriter);
	if (bCapturing && IMFSinkWriter == true)
	{
		SetWindowText(hButton, L"Stop IMFSinkWriter");
	}
	else
	{
		SetWindowText(hButton, L"Use IMFSinkWriter");
	}

	hButton = GetDlgItem(hDlg, IDC_VIEW);
	if (IsWindowVisible(g_hwnd) && IMFCaptureEnginePreview == true)
	{
		SetWindowText(hButton, L"Stop IMFCaptureEngine Preview");
	}
	else
	{
		SetWindowText(hButton, L"Use IMFCaptureEngine Preview");
	}

	EnableDialogControl(hDlg, IDC_CAPTURE_USING_IMFCaptureEngine, bCapturing || bEnable);
	EnableDialogControl(hDlg, IDC_VIDEO_AND_AUDIO_DEVICE_LIST, !bCapturing && bEnable);
}


//-----------------------------------------------------------------------------
// OnDeviceChange
// 
// Handles WM_DEVICECHANGE messages.
//-----------------------------------------------------------------------------

void OnDeviceChange(HWND hDlg, WPARAM reason, DEV_BROADCAST_HDR *pHdr)
{
	if (reason == DBT_DEVNODES_CHANGED || reason == DBT_DEVICEARRIVAL)
	{
		// Check for added/removed devices, regardless of whether
		// the application is capturing video at this time.

		UpdateDeviceList(hDlg);
		UpdateUI(hDlg);
	}

	// Now check if the current video capture device was lost.

	if (pHdr == NULL)
	{
		return;
	}
	if (pHdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
	{
		return;
	}

	HRESULT hr = S_OK;
	bool bDeviceLost = false;

	if (g_CaptureAPI->IsCapturing())
	{
		hr = g_CaptureAPI->CheckDeviceLost(pHdr, &bDeviceLost);
		if (FAILED(hr) || bDeviceLost)
		{
			StopCapture(hDlg);
			MessageBox(hDlg, L"The capture device was removed or lost.", L"Lost Device", MB_OK);
		}
	}
}


void NotifyError(HWND hwnd, const WCHAR *sErrorMessage, HRESULT hrErr)
{
	const size_t MESSAGE_LEN = 512;
	WCHAR message[MESSAGE_LEN];

	HRESULT hr = swprintf_s(message, MESSAGE_LEN, L"%s (HRESULT = 0x%X)", sErrorMessage, hrErr);
	if (SUCCEEDED(hr))
	{
		MessageBox(hwnd, message, NULL, MB_OK | MB_ICONERROR);
	}
}


void EnableDialogControl(HWND hDlg, int nIDDlgItem, bool bEnable)
{
	HWND hwnd = GetDlgItem(hDlg, nIDDlgItem);

	if (!bEnable &&  hwnd == GetFocus())
	{
		// When disabling a control that has focus, set the 
		// focus to the next control.

		::SendMessage(GetParent(hwnd), WM_NEXTDLGCTL, 0, FALSE);
	}
	EnableWindow(hwnd, bEnable);
}



