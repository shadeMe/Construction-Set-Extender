#pragma once
#include <BGSEERenderWindowFlyCamera.h>

namespace ConstructionSetExtender
{
	class CSERenderWindowFlyCameraOperator : public BGSEditorExtender::BGSEERenderWindowFlyCameraOperator
	{
		NiFrustum								ViewportFrustumBuffer;
	public:
		CSERenderWindowFlyCameraOperator(HWND Parent, BGSEditorExtender::ResourceTemplateT TemplateID);
		virtual ~CSERenderWindowFlyCameraOperator();

		virtual void							PrologCallback(void);
		virtual void							EpilogCallback(void);

		virtual void							Rotate(int XOffset, int YOffset);
		virtual void							Move(UInt8 Direction, bool Sprinting, bool Crawling);

		virtual void							RefreshRenderWindow(void);
	};
}