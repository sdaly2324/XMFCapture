#include <memory>
#include <atlcomcli.h>

class XMFCaptureDevice;
class XMFCaptureEngineRep;
struct IMFActivate;
class XMFCaptureEngine
{
public:
	XMFCaptureEngine(std::shared_ptr<XMFCaptureDevice> pAudioDevice, std::shared_ptr<XMFCaptureDevice> pVideoDevice, bool useOld);
	virtual ~XMFCaptureEngine();

	HRESULT StartPreview(HWND hwnd);
	HRESULT StopPreview();

	HRESULT StartRecord(PCWSTR pszDestinationFile);
	HRESULT StopRecord();

	void SleepState(bool fSleeping);

	bool IsPreviewing() const;
	bool IsRecording() const;
	HRESULT get_FramesCaptured(unsigned long* pVal) const;
	HRESULT	get_FPSForCapture(long* pVal) const;

private:
	XMFCaptureEngine();

	XMFCaptureEngineRep* m_RepPtr;
};