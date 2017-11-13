#include "VideoDisplayControl.h"
#include "MFUtils.h"

#include <evr.h>

class VideoDisplayControlRep : public MFUtils
{
public:
	VideoDisplayControlRep(HWND windowForVideo);
	~VideoDisplayControlRep();

	HRESULT								GetLastHRESULT();

	void								OnTopologyReady(CComPtr<IMFMediaSession> mediaSession);

private:
	CComPtr<IMFVideoDisplayControl>		mVideoDisplayControl = nullptr;
	HWND mWindowForVideo = nullptr;
};

VideoDisplayControl::VideoDisplayControl(HWND windowForVideo)
{
	m_pRep = std::unique_ptr<VideoDisplayControlRep>(new VideoDisplayControlRep(windowForVideo));
}
VideoDisplayControlRep::VideoDisplayControlRep(HWND windowForVideo) :
	mWindowForVideo(windowForVideo)
{
}
VideoDisplayControl::~VideoDisplayControl()
{
}
VideoDisplayControlRep::~VideoDisplayControlRep()
{
}

HRESULT VideoDisplayControl::GetLastHRESULT()
{
	return m_pRep->GetLastHRESULT();
}
HRESULT VideoDisplayControlRep::GetLastHRESULT()
{
	return MFUtils::GetLastHRESULT();
}

void VideoDisplayControl::OnTopologyReady(CComPtr<IMFMediaSession> mediaSession)
{
	m_pRep->OnTopologyReady(mediaSession);
}
void VideoDisplayControlRep::OnTopologyReady(CComPtr<IMFMediaSession> mediaSession)
{
	if (mWindowForVideo)
	{
		mVideoDisplayControl.Release();
		OnERR_return(MFGetService(mediaSession, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&mVideoDisplayControl));
		OnERR_return(mVideoDisplayControl->SetVideoWindow(mWindowForVideo));
	}
}