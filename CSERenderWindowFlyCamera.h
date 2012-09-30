#pragma once
#include <BGSEERenderWindowFlyCamera.h>

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		enum
		{
			kRenderWindowFlyCamera_MovementSpeed = 0,
			kRenderWindowFlyCamera_SprintMultiplier,
			kRenderWindowFlyCamera_CrawlMultiplier,
			kRenderWindowFlyCamera_RotationSpeed,
			kRenderWindowFlyCamera_CameraFOV,

			kRenderWindowFlyCamera__MAX
		};
		extern const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kRenderWindowFlyCameraINISettings[kRenderWindowFlyCamera__MAX];
		BGSEditorExtender::BGSEEINIManagerSettingFactory*								GetRenderWindowFlyCamera(void);
	}

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