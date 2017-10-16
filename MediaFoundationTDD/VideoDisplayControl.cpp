#include "VideoDisplayControl.h"
#include "IMFWrapper.h"

#include <evr.h>

class VideoDisplayControlRep : public IMFWrapper
{
public:
	VideoDisplayControlRep();
	~VideoDisplayControlRep();

	void								OnTopologyReady(CComPtr<IMFMediaSession> mediaSession);

	HRESULT								GetLastHRESULT();

	CComPtr<IMFVideoDisplayControl>		GetVideoDisplayControl();

private:
	CComPtr<IMFVideoDisplayControl>		mVideoDisplayControl = NULL;
};

VideoDisplayControl::VideoDisplayControl()
{
	m_pRep = std::unique_ptr<VideoDisplayControlRep>(new VideoDisplayControlRep());
}
VideoDisplayControlRep::VideoDisplayControlRep()
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
	return IMFWrapper::GetLastHRESULT();
}

CComPtr<IMFVideoDisplayControl> VideoDisplayControl::GetVideoDisplayControl()
{
	return m_pRep->GetVideoDisplayControl();
}
CComPtr<IMFVideoDisplayControl> VideoDisplayControlRep::GetVideoDisplayControl()
{
	return mVideoDisplayControl;
}

void VideoDisplayControl::OnTopologyReady(CComPtr<IMFMediaSession> mediaSession)
{
	m_pRep->OnTopologyReady(mediaSession);
}
void VideoDisplayControlRep::OnTopologyReady(CComPtr<IMFMediaSession> mediaSession)
{
	mVideoDisplayControl.Release();
	PrintIfErrAndSave(MFGetService(mediaSession, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&mVideoDisplayControl));
}