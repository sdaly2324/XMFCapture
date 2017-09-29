#include "stdafx.h"

#include "XMFCaptureDeviceManager.h"
#include "XMFUtilities.h"
#include "XMFCaptureDevice.h"
#include "XMFFormat.h"

#include <mfapi.h>
#include <mfidl.h>

class XMFCaptureDeviceManagerRep
{
public:
	XMFCaptureDeviceManagerRep();
	virtual ~XMFCaptureDeviceManagerRep();

	bool AnyVideoOnlyDevices() const;
	bool AnyAudioOnlyDevices() const;
	bool AnyVideoAndAudioDevices() const;

	void    Clear();
	HRESULT ReEnumerateDevices();

	XOSStringList GetDevicePairNamesList();
	void GetDeviceByName(XOSString deviceName, std::shared_ptr<XMFCaptureDevice>* videoDevice, std::shared_ptr<XMFCaptureDevice>* audioDevice);
	void GetDefaultDevices(std::shared_ptr<XMFCaptureDevice>* videoDevice, std::shared_ptr<XMFCaptureDevice>* audioDevice);
	XOSString GetDevcePairNameWithPreFix(std::pair < DEVICE_PAIR > &devPair);

private:
	HRESULT EnumerateDeviceType(bool audio);

	void AddDeviceToTheVideoOnlyList(std::shared_ptr<XMFCaptureDevice> XMFCaptureDevicePtr);
	void AddDeviceToTheAudioOnlyList(std::shared_ptr<XMFCaptureDevice> XMFCaptureDevicePtr);
	void AddDeviceToTheAudioAndVideoList(std::shared_ptr<XMFCaptureDevice> XMFCaptureDevicePtr);
	void CreateDesiredFormatLists();
	void AddVideoFormatWithAttrValues(XOSString frameSize, XOSString frameRate, XOSString uncompressedFormat);
	void CreateDesiredVideoFormatLists();
	void AddAudioFormatWithAttrValues(XOSString sampleRate, XOSString numChan, XOSString bitsPerSample, XOSString sampleType);
	void CreateDesiredAudioFormatLists();
	void CreateBlackListedDeviceNameList();
	bool IsDeviceBlackListed(std::shared_ptr<XMFCaptureDevice> XMFCaptureDevicePtr);
	void CreateXMFDevicePairList();
	void JustXI100DUSB();
	void JustBlackmagicWDM();
	void JustWebCam();

	DEVICE_PAIRS										m_XMFDevicePairList;
	std::vector<std::shared_ptr<XMFCaptureDevice>>	m_XMFDeviceVideoOnlyList;
	std::vector<std::shared_ptr<XMFCaptureDevice>>	m_XMFDeviceAudioOnlyList;
	std::vector<std::shared_ptr<XMFCaptureDevice>>	m_XMFDeviceAudioAndVideoList;
	std::vector<std::shared_ptr<XMFCaptureDevice>>	m_UnsupportedDeviceList;

	std::vector<std::shared_ptr<XMFFormat>>			m_DesiredVideoFormatList;
	std::vector<std::shared_ptr<XMFFormat>>			m_DesiredAudioFormatList;
	XOSStringList										m_BlackListedDeviceNameList;

	std::pair < DEVICE_PAIR >							m_SelectedDevicesPair;
};

XMFCaptureDeviceManager::XMFCaptureDeviceManager()
{
	m_pRep = new XMFCaptureDeviceManagerRep();
}
XMFCaptureDeviceManagerRep::XMFCaptureDeviceManagerRep() :
m_SelectedDevicesPair(NULL, NULL)
{
	// Initialize the COM library
	SUCCEEDED_Xv(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));

	m_XMFDeviceVideoOnlyList.clear();
	m_XMFDeviceAudioOnlyList.clear();
	m_XMFDeviceAudioAndVideoList.clear();

	CreateDesiredFormatLists();
	CreateBlackListedDeviceNameList();
}

XMFCaptureDeviceManager::~XMFCaptureDeviceManager()
{
	if (m_pRep)
	{
		delete m_pRep;
		m_pRep = NULL;
	}
}
XMFCaptureDeviceManagerRep::~XMFCaptureDeviceManagerRep()
{
	Clear();
}

void XMFCaptureDeviceManager::Clear()
{
	if (m_pRep)
	{
		m_pRep->Clear();
	}
}
void XMFCaptureDeviceManagerRep::Clear()
{
	m_XMFDevicePairList.clear();
	m_XMFDeviceVideoOnlyList.clear();
	m_XMFDeviceAudioOnlyList.clear();
	m_XMFDeviceAudioAndVideoList.clear();
	m_UnsupportedDeviceList.clear();
}

HRESULT XMFCaptureDeviceManager::ReEnumerateDevices()
{
	if (m_pRep)
	{
		return m_pRep->ReEnumerateDevices();
	}
	return E_FAIL;
}
HRESULT XMFCaptureDeviceManagerRep::ReEnumerateDevices()
{
	HRESULT hr = S_OK;
	Clear();

	// VIDEO
	if (SUCCEEDED_Xb(hr))
	{
		hr = EnumerateDeviceType(false);
	}

	// AUDIO
	if (SUCCEEDED_Xb(hr))
	{
		hr = EnumerateDeviceType(true);
	}

	if (SUCCEEDED_Xb(hr))
	{
		CreateXMFDevicePairList();
	}

	return hr;
}

bool IsDupeDevice(std::vector<std::shared_ptr<XMFCaptureDevice>>* listPtr, std::shared_ptr<XMFCaptureDevice> XMFCaptureDevicePtr)
{
	bool foundDupe = false;
	for (auto& device : *listPtr)
	{
		if (*device == *XMFCaptureDevicePtr)
		{
			foundDupe = true;
			break;
		}
	}
	return foundDupe;
}

void XMFCaptureDeviceManagerRep::AddDeviceToTheVideoOnlyList(std::shared_ptr<XMFCaptureDevice> XMFCaptureDevicePtr)
{
	if (!IsDupeDevice(&m_XMFDeviceVideoOnlyList, XMFCaptureDevicePtr))
	{
		if (!IsDeviceBlackListed(XMFCaptureDevicePtr) && XMFCaptureDevicePtr->SupportsAnyOfTheseFormats(m_DesiredVideoFormatList))
		{
			m_XMFDeviceVideoOnlyList.push_back(XMFCaptureDevicePtr);
		}
		else
		{
			m_UnsupportedDeviceList.push_back(XMFCaptureDevicePtr);
		}
	}
}

void XMFCaptureDeviceManagerRep::AddDeviceToTheAudioOnlyList(std::shared_ptr<XMFCaptureDevice> XMFCaptureDevicePtr)
{
	if (!IsDupeDevice(&m_XMFDeviceAudioOnlyList, XMFCaptureDevicePtr))
	{
		if (!IsDeviceBlackListed(XMFCaptureDevicePtr) && XMFCaptureDevicePtr->SupportsAnyOfTheseFormats(m_DesiredAudioFormatList))
		{
			m_XMFDeviceAudioOnlyList.push_back(XMFCaptureDevicePtr);
		}
		else
		{
			m_UnsupportedDeviceList.push_back(XMFCaptureDevicePtr);
		}
		
	}
}

void XMFCaptureDeviceManagerRep::AddDeviceToTheAudioAndVideoList(std::shared_ptr<XMFCaptureDevice> XMFCaptureDevicePtr)
{
	if (!IsDupeDevice(&m_XMFDeviceAudioAndVideoList, XMFCaptureDevicePtr))
	{
		bool hasAGoodVideoFormat = !IsDeviceBlackListed(XMFCaptureDevicePtr) && XMFCaptureDevicePtr->SupportsAnyOfTheseFormats(m_DesiredVideoFormatList);
		bool hasAGoodAudioFormat = !IsDeviceBlackListed(XMFCaptureDevicePtr) && XMFCaptureDevicePtr->SupportsAnyOfTheseFormats(m_DesiredAudioFormatList);
		
		if (hasAGoodVideoFormat && hasAGoodAudioFormat)
		{
			m_XMFDeviceAudioAndVideoList.push_back(XMFCaptureDevicePtr);
		}
		else if (hasAGoodVideoFormat)
		{
			m_XMFDeviceVideoOnlyList.push_back(XMFCaptureDevicePtr);
		}
		else if (hasAGoodAudioFormat)
		{
			m_XMFDeviceAudioOnlyList.push_back(XMFCaptureDevicePtr);
		}
		else
		{
			m_UnsupportedDeviceList.push_back(XMFCaptureDevicePtr);
		}
	}
}

HRESULT XMFCaptureDeviceManagerRep::EnumerateDeviceType(bool audio)
{
	HRESULT hr = S_OK;
	CComPtr<IMFAttributes> pAttributes = NULL;

	hr = MFCreateAttributes(&pAttributes, 1);

	if (SUCCEEDED_Xb(hr))
	{
		if (audio)
		{
			hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
		}
		else
		{
			hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		}
	}

	if (SUCCEEDED_Xb(hr))
	{
		if (audio)
		{
			IMFActivate** listpAudioDevices = 0;
			UINT32 numAudioDevices = 0;
			hr = MFEnumDeviceSources(pAttributes, &listpAudioDevices, &numAudioDevices);
			for (UINT32 i = 0; i < numAudioDevices; i++)
			{
				auto audioDevicePtr = std::make_shared<XMFCaptureDevice>(listpAudioDevices[i], MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
				if (audioDevicePtr->SupportsAudioAndVideo())
				{
					AddDeviceToTheAudioAndVideoList(audioDevicePtr);
				}
				else
				{
					AddDeviceToTheAudioOnlyList(audioDevicePtr);
				}
			}
		}
		else
		{
			IMFActivate** listpVideoDevices = 0;
			UINT32 numVideoDevices = 0;
			hr = MFEnumDeviceSources(pAttributes, &listpVideoDevices, &numVideoDevices);
			for (UINT32 i = 0; i < numVideoDevices; i++)
			{
				auto videoDevicePtr = std::make_shared<XMFCaptureDevice>(listpVideoDevices[i], MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
				if (videoDevicePtr->SupportsAudioAndVideo())
				{
					AddDeviceToTheAudioAndVideoList(videoDevicePtr);
				}
				else
				{
					AddDeviceToTheVideoOnlyList(videoDevicePtr);
				}
			}
		}
	}

	return hr;
}

bool XMFCaptureDeviceManager::AnyVideoOnlyDevices() const
{
	if (m_pRep)
	{
		return m_pRep->AnyVideoOnlyDevices();
	}
	return false;
}
bool XMFCaptureDeviceManagerRep::AnyVideoOnlyDevices() const
{ 
	if (m_XMFDeviceVideoOnlyList.empty())
	{
		return false;
	}
	return true; 
}

bool XMFCaptureDeviceManager::AnyAudioOnlyDevices() const
{
	if (m_pRep)
	{
		return m_pRep->AnyAudioOnlyDevices();
	}
	return false;
}
bool XMFCaptureDeviceManagerRep::AnyAudioOnlyDevices() const
{
	if (m_XMFDeviceAudioOnlyList.empty())
	{
		return false;
	}
	return true;
}

bool XMFCaptureDeviceManager::AnyVideoAndAudioDevices() const
{
	if (m_pRep)
	{
		return m_pRep->AnyVideoAndAudioDevices();
	}
	return false;
}
bool XMFCaptureDeviceManagerRep::AnyVideoAndAudioDevices() const
{
	if (m_XMFDeviceAudioAndVideoList.empty())
	{
		if (m_XMFDeviceVideoOnlyList.empty() || m_XMFDeviceAudioOnlyList.empty())
		{
			return false;
		}
	}
	return true;
}

XOSString DeviceNameWithAVSupport(std::shared_ptr<XMFCaptureDevice> devicePtr)
{
	WCHAR name[1024];
	wcscpy_s(name, L"A/V - ");
	wcsncat_s(name, devicePtr->GetDeviceName(), 1024);
	XOSString reVal(new std::wstring(name));
	return reVal;
}

XOSString GetDevceNameWithPreFix(std::shared_ptr<XMFCaptureDevice> devicePtr, bool audio)
{
	WCHAR name[1024];

	if (devicePtr)
	{
		if (audio)
		{
			wcscpy_s(name, L"A - ");
		}
		else
		{
			wcscpy_s(name, L"V - ");
		}
		wcsncat_s(name, devicePtr->GetDeviceName(), 1024);
	}
	else
	{
		wcscpy_s(name, L"GetDevceNameWithPreFix UNKNOWN");
	}

	XOSString reVal(new std::wstring(name));
	return reVal;
}

XOSString XMFCaptureDeviceManager::GetDevcePairNameWithPreFix(std::pair < DEVICE_PAIR > &devPair)
{
	if (m_pRep)
	{
		return m_pRep->GetDevcePairNameWithPreFix(devPair);
	}

	XOSString reVal(new std::wstring(L""));
	return reVal;
}
XOSString XMFCaptureDeviceManagerRep::GetDevcePairNameWithPreFix(std::pair < DEVICE_PAIR > &devPair)
{
	std::shared_ptr<XMFCaptureDevice> fPtr = devPair.first;
	std::shared_ptr<XMFCaptureDevice> sPtr = devPair.second;
	if (fPtr && sPtr)
	{
		if (fPtr->SupportsAudioAndVideo())
		{
			return DeviceNameWithAVSupport(fPtr);
		}
		else if (sPtr->SupportsAudioAndVideo())
		{
			return DeviceNameWithAVSupport(sPtr);
		}
		else if (fPtr->SupportsAudio() && sPtr->SupportsVideo())
		{
			return DeviceNameWithAVSupport(sPtr);
		}
		else if (fPtr->SupportsVideo() && sPtr->SupportsAudio())
		{
			return DeviceNameWithAVSupport(fPtr);
		}
	}

	if (fPtr) // assume first is video and second is audio
	{
		return GetDevceNameWithPreFix(fPtr, false);
	}
	else if (sPtr)
	{
		return GetDevceNameWithPreFix(sPtr, true);
	}

	return GetDevceNameWithPreFix(NULL, false);
}

XOSStringList XMFCaptureDeviceManager::GetDevicePairNamesList()
{
	if (m_pRep)
	{
		return m_pRep->GetDevicePairNamesList();
	}
	XOSStringList emptyList;
	return emptyList;
}
XOSStringList XMFCaptureDeviceManagerRep::GetDevicePairNamesList()
{
	XOSStringList deviceNamesList;
	for (auto& devicePair : m_XMFDevicePairList)
	{
		deviceNamesList.push_back(GetDevcePairNameWithPreFix(devicePair));
	}
	return deviceNamesList;
}

void XMFCaptureDeviceManager::GetDefaultDevices(std::shared_ptr<XMFCaptureDevice>* videoDevice, std::shared_ptr<XMFCaptureDevice>* audioDevice)
{
	if (m_pRep)
	{
		m_pRep->GetDefaultDevices(videoDevice, audioDevice);
	}	
}
void XMFCaptureDeviceManagerRep::GetDefaultDevices(std::shared_ptr<XMFCaptureDevice>* videoDevice, std::shared_ptr<XMFCaptureDevice>* audioDevice)
{
	if (videoDevice && audioDevice && m_XMFDevicePairList.size() > 0)
	{
		*videoDevice = m_XMFDevicePairList[0].first;
		*audioDevice = m_XMFDevicePairList[0].second;
	}
}

void XMFCaptureDeviceManager::GetDeviceByName(XOSString deviceName, std::shared_ptr<XMFCaptureDevice>* videoDevice, std::shared_ptr<XMFCaptureDevice>* audioDevice)
{
	if (m_pRep)
	{
		m_pRep->GetDeviceByName(deviceName, videoDevice, audioDevice);
	}
}
void XMFCaptureDeviceManagerRep::GetDeviceByName(XOSString deviceName, std::shared_ptr<XMFCaptureDevice>* videoDevice, std::shared_ptr<XMFCaptureDevice>* audioDevice)
{
	for (auto& devicePair : m_XMFDevicePairList)
	{
		XOSString deviceImLookingFor = GetDevcePairNameWithPreFix(devicePair);
		if (deviceName->length() == deviceImLookingFor->length() && std::equal(deviceName->begin(), deviceName->end(), deviceImLookingFor->begin()))
		{
			if (videoDevice)
			{
				*videoDevice = devicePair.first;
			}
			if (audioDevice)
			{
				*audioDevice = devicePair.second;
			}
			break;
		}
	}
}

void XMFCaptureDeviceManagerRep::CreateDesiredFormatLists()
{
	CreateDesiredVideoFormatLists();
	CreateDesiredAudioFormatLists();
}

void XMFCaptureDeviceManagerRep::AddVideoFormatWithAttrValues(XOSString frameSize, XOSString frameRate, XOSString uncompressedFormat)
{
	CComPtr<IMFActivate> NULLIMFActivate = NULL;
	auto desiredVideoFormat = std::make_shared<XMFFormat>(NULLIMFActivate, LONG_MAX, LONG_MAX);
	desiredVideoFormat->AddAtribute(MF_MT_FRAME_SIZE, frameSize->c_str());
	desiredVideoFormat->AddAtribute(MF_MT_FRAME_RATE, frameRate->c_str());
	desiredVideoFormat->AddAtribute(MF_MT_SUBTYPE, uncompressedFormat->c_str());
	m_DesiredVideoFormatList.push_back(desiredVideoFormat);
}

void XMFCaptureDeviceManagerRep::CreateDesiredVideoFormatLists()
{
	m_DesiredVideoFormatList.clear();

	// 720p 59.94, v210
	{
		XOSString frameSize(new std::wstring(L"1280/720"));
		XOSString frameRate(new std::wstring(L"60000/1001"));
		XOSString uncompressedFormat(new std::wstring(L"MFVideoFormat_v210"));
		AddVideoFormatWithAttrValues(frameSize, frameRate, uncompressedFormat);
	}

	// 1080i 59.94, v210
	{
		XOSString frameSize(new std::wstring(L"1920/1080"));
		XOSString frameRate(new std::wstring(L"30000/1001"));
		XOSString uncompressedFormat(new std::wstring(L"MFVideoFormat_v210"));
		AddVideoFormatWithAttrValues(frameSize, frameRate, uncompressedFormat);
	}

	// 1080p 59.94, v210 (currently not seen in BM supported format list????)
	{
		XOSString frameSize(new std::wstring(L"1920/1080"));
		XOSString frameRate(new std::wstring(L"60000/1001"));
		XOSString uncompressedFormat(new std::wstring(L"MFVideoFormat_v210"));
		AddVideoFormatWithAttrValues(frameSize, frameRate, uncompressedFormat);
	}

	// 480i 30, YUY2 (Webcam)
	{
		XOSString frameSize(new std::wstring(L"640/480"));
		XOSString frameRate(new std::wstring(L"30/1"));
		XOSString uncompressedFormat(new std::wstring(L"MFVideoFormat_YUY2"));
		AddVideoFormatWithAttrValues(frameSize, frameRate, uncompressedFormat);
	}

	// 720p 59.94, YUY2
	{
		XOSString frameSize(new std::wstring(L"1280/720"));
		XOSString frameRate(new std::wstring(L"60000/1001"));
		XOSString uncompressedFormat(new std::wstring(L"MFVideoFormat_YUY2"));
		AddVideoFormatWithAttrValues(frameSize, frameRate, uncompressedFormat);
	}

	// 1080i 59.94, YUY2
	{
		XOSString frameSize(new std::wstring(L"1920/1080"));
		XOSString frameRate(new std::wstring(L"30000/1001"));
		XOSString uncompressedFormat(new std::wstring(L"MFVideoFormat_YUY2"));
		AddVideoFormatWithAttrValues(frameSize, frameRate, uncompressedFormat);
	}

	// 1080p 59.94, YUY2 (currently not seen in BM supported format list????)
	{
		XOSString frameSize(new std::wstring(L"1920/1080"));
		XOSString frameRate(new std::wstring(L"60000/1001"));
		XOSString uncompressedFormat(new std::wstring(L"MFVideoFormat_YUY2"));
		AddVideoFormatWithAttrValues(frameSize, frameRate, uncompressedFormat);
	}
}

void XMFCaptureDeviceManagerRep::AddAudioFormatWithAttrValues(XOSString sampleRate, XOSString numChan, XOSString bitsPerSample, XOSString sampleType)
{
	CComPtr<IMFActivate> NULLIMFActivate = NULL;
	auto desiredAudioFormat = std::make_shared<XMFFormat>(NULLIMFActivate, LONG_MAX, LONG_MAX);
	desiredAudioFormat->AddAtribute(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate->c_str());
	desiredAudioFormat->AddAtribute(MF_MT_AUDIO_NUM_CHANNELS, numChan->c_str());
	desiredAudioFormat->AddAtribute(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample->c_str());
	desiredAudioFormat->AddAtribute(MF_MT_SUBTYPE, sampleType->c_str());
	m_DesiredAudioFormatList.push_back(desiredAudioFormat);
}

void XMFCaptureDeviceManagerRep::CreateDesiredAudioFormatLists()
{
	m_DesiredAudioFormatList.clear();

	// 48k, 2 channels, 16 bit PCM
	{
		std::shared_ptr<int> sp1(new int(5));
		XOSString sampleRate(new std::wstring(L"48000"));
		XOSString numChan(new std::wstring(L"2"));
		XOSString bitsPerSample(new std::wstring(L"16"));
		XOSString sampleType(new std::wstring(L"MFAudioFormat_PCM"));
		AddAudioFormatWithAttrValues(sampleRate, numChan, bitsPerSample, sampleType);
	}

	// 48k, 2 channels, 32 bit float
	{
		XOSString sampleRate(new std::wstring(L"48000"));
		XOSString numChan(new std::wstring(L"2"));
		XOSString bitsPerSample(new std::wstring(L"32"));
		XOSString sampleType(new std::wstring(L"MFAudioFormat_Float"));
		AddAudioFormatWithAttrValues(sampleRate, numChan, bitsPerSample, sampleType);
	}
}

void XMFCaptureDeviceManagerRep::CreateBlackListedDeviceNameList()
{
	//m_BlackListedDeviceNameList.push_back(L"Line In (Blackmagic Audio)");
}

bool XMFCaptureDeviceManagerRep::IsDeviceBlackListed(std::shared_ptr<XMFCaptureDevice> XMFCaptureDevicePtr)
{
	for (auto& badDeviceName : m_BlackListedDeviceNameList)
	{
		if (_wcsicmp(XMFCaptureDevicePtr->GetDeviceName(), badDeviceName->c_str()) == 0)
		{
			return true;
		}
	}

	return false;
}

void XMFCaptureDeviceManagerRep::JustBlackmagicWDM()
{
	bool useSingleDevice = false;
	size_t index = 0;
	for (; index < m_XMFDeviceAudioAndVideoList.size(); index++)
	{
		if (wcsstr(m_XMFDeviceAudioAndVideoList[index]->GetDeviceName(), L"Blackmagic WDM Capture") != 0)
		{
			if (useSingleDevice)
			{
				m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceAudioAndVideoList[index], m_XMFDeviceAudioAndVideoList[index]));
				//m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceAudioAndVideoList[index], NULL));
				//m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(NULL, m_XMFDeviceAudioAndVideoList[index]));
			}
			else
			{
				size_t index2 = 0;
				for (; index2 < m_XMFDeviceAudioOnlyList.size(); index2++)
				{
					if (wcsstr(m_XMFDeviceAudioOnlyList[index2]->GetDeviceName(), L"Blackmagic Audio") != 0)
					{
						m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceAudioAndVideoList[index], m_XMFDeviceAudioOnlyList[index2]));
						//m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(NULL, m_XMFDeviceAudioOnlyList[index2]));
					}
				}
				//m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceAudioAndVideoList[index], NULL));
			}
		}
	}
}

void XMFCaptureDeviceManagerRep::JustXI100DUSB()
{
	size_t index = 0;
	for (; index < m_XMFDeviceVideoOnlyList.size() && index < m_XMFDeviceAudioOnlyList.size(); index++)
	{
		if (wcsstr(m_XMFDeviceVideoOnlyList[index]->GetDeviceName(), L"XI100DUSB-") != 0)
		{
			size_t audioIndex = 0;
			for (; audioIndex < m_XMFDeviceAudioOnlyList.size(); audioIndex++)
			{
				if (wcsstr(m_XMFDeviceAudioOnlyList[audioIndex]->GetDeviceName(), L"XI100DUSB-") != 0)
				{
					m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceVideoOnlyList[index], m_XMFDeviceAudioOnlyList[audioIndex]));
					//m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceVideoOnlyList[index], NULL));
					//m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(NULL, m_XMFDeviceAudioOnlyList[audioIndex]));
					break;
				}
			}
		}
	}
}

void XMFCaptureDeviceManagerRep::JustWebCam()
{
	size_t index = 0;
	for (; index < m_XMFDeviceVideoOnlyList.size() && index < m_XMFDeviceAudioOnlyList.size(); index++)
	{
		if (wcsstr(m_XMFDeviceVideoOnlyList[index]->GetDeviceName(), L"Webcam") != 0)
		{
			size_t audioIndex = 0;
			for (; audioIndex < m_XMFDeviceAudioOnlyList.size(); audioIndex++)
			{
				if (wcsstr(m_XMFDeviceAudioOnlyList[audioIndex]->GetDeviceName(), L"Microphone") != 0)
				{
					m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceVideoOnlyList[index], m_XMFDeviceAudioOnlyList[audioIndex]));
					m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceVideoOnlyList[index], NULL));
					m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(NULL, m_XMFDeviceAudioOnlyList[audioIndex]));
					break;
				}
			}
		}
	}
}
void XMFCaptureDeviceManagerRep::CreateXMFDevicePairList()
{
	//JustWebCam();
	//JustXI100DUSB();
	//JustBlackmagicWDM();
	//return;
	// add all the devices that have both
	for (auto& device : m_XMFDeviceAudioAndVideoList)
	{
		m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(device, device));

		// split them
		//m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(device, NULL));
		//m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(NULL, device));
	}

	// pair up the only video with the only audio devices
	size_t index = 0;
	for (; index < m_XMFDeviceVideoOnlyList.size() && index < m_XMFDeviceAudioOnlyList.size(); index++)
	{
		// web cam goes with the microphone
		if (wcsstr(m_XMFDeviceVideoOnlyList[index]->GetDeviceName(), L"Webcam") != 0)
		{
			size_t audioIndex = 0;
			for (; audioIndex < m_XMFDeviceAudioOnlyList.size(); audioIndex++)
			{
				if (wcsstr(m_XMFDeviceAudioOnlyList[audioIndex]->GetDeviceName(), L"Microphone") != 0)
				{
					m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceVideoOnlyList[index], m_XMFDeviceAudioOnlyList[audioIndex]));
					break;
				}
			}
		}
		// XI100DUSB-SDI
		else if (wcsstr(m_XMFDeviceVideoOnlyList[index]->GetDeviceName(), L"XI100DUSB-SDI") != 0)
		{
			size_t audioIndex = 0;
			for (; audioIndex < m_XMFDeviceAudioOnlyList.size(); audioIndex++)
			{
				if (wcsstr(m_XMFDeviceAudioOnlyList[audioIndex]->GetDeviceName(), L"XI100DUSB-SDI") != 0)
				{
					m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceVideoOnlyList[index], m_XMFDeviceAudioOnlyList[audioIndex]));
					break;
				}
			}
		}
	}
	index = 0;
	// video only
	for (; index < m_XMFDeviceVideoOnlyList.size(); index++)
	{
		m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(m_XMFDeviceVideoOnlyList[index], NULL));
	}
	index = 0;
	// audio only
	for (; index < m_XMFDeviceAudioOnlyList.size(); index++)
	{
		m_XMFDevicePairList.push_back(std::pair<DEVICE_PAIR>(NULL, m_XMFDeviceAudioOnlyList[index]));
	}
	
}