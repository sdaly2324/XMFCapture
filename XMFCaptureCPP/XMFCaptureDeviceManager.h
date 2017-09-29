#pragma once

#include <windows.h>
#include <vector>
#include <memory>

#include "XMFUtilities.h"

class XMFCaptureDevice;

#define DEVICE_PAIR std::shared_ptr<XMFCaptureDevice>, std::shared_ptr<XMFCaptureDevice>
#define DEVICE_PAIRS std::vector<std::pair<DEVICE_PAIR>>

class XMFCaptureDeviceManagerRep;
class XMFCaptureDeviceManager
{
public:
	XMFCaptureDeviceManager();
	virtual ~XMFCaptureDeviceManager();

	bool XMFCaptureDeviceManager::AnyVideoOnlyDevices() const;
	bool XMFCaptureDeviceManager::AnyAudioOnlyDevices() const;
	bool XMFCaptureDeviceManager::AnyVideoAndAudioDevices() const;

	void    Clear();
	HRESULT ReEnumerateDevices();

	XOSStringList GetDevicePairNamesList();
	void GetDeviceByName(XOSString deviceName, std::shared_ptr<XMFCaptureDevice>* videoDevice, std::shared_ptr<XMFCaptureDevice>* audioDevice);
	void GetDefaultDevices(std::shared_ptr<XMFCaptureDevice>* videoDevice, std::shared_ptr<XMFCaptureDevice>* audioDevice);
	XOSString GetDevcePairNameWithPreFix(std::pair < DEVICE_PAIR > &devPair);

private:
	XMFCaptureDeviceManagerRep* m_pRep;
};

