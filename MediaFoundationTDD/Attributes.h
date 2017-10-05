#pragma once

#ifdef MediaFoundationTDD_EXPORTS
#define MediaFoundationTDD_API __declspec(dllexport)
#else
#define MediaFoundationTDD_API __declspec(dllimport)
#endif

struct IMFAttributes;
class AttributesRep;
class MediaFoundationTDD_API Attributes
{
public:
	Attributes();
	~Attributes();

	IMFAttributes*	GetAttributes();

	void			CreateVideoDeviceAttributes();

private:
	AttributesRep* m_pRep = 0;
};