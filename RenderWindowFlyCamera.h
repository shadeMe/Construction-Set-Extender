#pragma once
#include <bgsee\RenderWindowFlyCamera.h>

namespace cse
{
	class RenderWindowFlyCameraOperator : public bgsee::RenderWindowFlyCameraOperator
	{
		NiFrustum								ViewportFrustumBuffer;
	public:
		RenderWindowFlyCameraOperator(HWND Parent, bgsee::ResourceTemplateT TemplateID);
		virtual ~RenderWindowFlyCameraOperator();

		virtual void							PrologCallback(void);
		virtual void							EpilogCallback(void);

		virtual void							Rotate(int XOffset, int YOffset);
		virtual void							Move(UInt8 Direction, bool Sprinting, bool Crawling);

		virtual void							RefreshRenderWindow(void);
	};
}