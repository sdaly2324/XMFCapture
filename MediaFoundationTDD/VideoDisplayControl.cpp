#include "VideoDisplayControl.h"
#include "MFUtils.h"

#include <evr.h>

class VideoDisplayControlRep : public MFUtils
{
public:
	VideoDisplayControlRep();
	~VideoDisplayControlRep();
	void SetIsParticipating(bool isParticipating);

	void								OnTopologyReady(CComPtr<IMFMediaSession> mediaSession);

	HRESULT								GetLastHRESULT();

	CComPtr<IMFVideoDisplayControl>		GetVideoDisplayControl();

private:
	CComPtr<IMFVideoDisplayControl>		mVideoDisplayControl = NULL;
	bool mIsParticipating = true;
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
	return MFUtils::GetLastHRESULT();
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
	if (mIsParticipating)
	{
		mVideoDisplayControl.Release();
		OnERR_return(MFGetService(mediaSession, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, (void**)&mVideoDisplayControl));
	}
}

void VideoDisplayControl::SetIsParticipating(bool isParticipating)
{
	m_pRep->SetIsParticipating(isParticipating);
}
void VideoDisplayControlRep::SetIsParticipating(bool isParticipating)
{
	mIsParticipating = isParticipating;
}