#pragma  once

namespace cse
{
	class AuxiliaryViewport : public bgsee::GenericModelessDialog
	{
		static AuxiliaryViewport*			Singleton;
	protected:
		static LRESULT CALLBACK				BaseDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);

		static const char*					kWindowTitle;

		static INISetting					kINI_Top;
		static INISetting					kINI_Left;
		static INISetting					kINI_Right;
		static INISetting					kINI_Bottom;
		static INISetting					kINI_Visible;
		static INISetting					kINI_PanSpeed;
		static INISetting					kINI_ZoomSpeed;
		static INISetting					kINI_RotationSpeed;

		class GlobalEventSink : public SME::MiscGunk::IEventSink
		{
			AuxiliaryViewport*				Parent;
		public:
			GlobalEventSink(AuxiliaryViewport* Parent);

			virtual void					Handle(SME::MiscGunk::IEventData* Data);
		};

		NiNode*								CameraRoot;
		NiCamera*							ViewportCamera;
		bool								Panning;
		bool								Zooming;
		bool								Rotating;
		POINT								LastMouseCoords;
		bool								Rendering;
		GlobalEventSink*					EventSink;

		void								BeginMouseCapture(HWND hWnd, bool& StateFlag);
		void								EndMouseCapture(bool& StateFlag);
		void								DrawBackBuffer(void);
		void								SyncWithPrimaryCamera();

		AuxiliaryViewport();
	public:
		virtual ~AuxiliaryViewport();

		static AuxiliaryViewport*			GetSingleton();
		void								Initialize();

		void								SetCameraFOV(float FOV);
		void								Draw(NiNode* NodeToRender, NiCamera* Camera);
		void								Redraw();
		void								ClearScreen();
		bool								IsRenderingPerspective() const;
		virtual bool						ToggleVisibility() override;

		static void							RegisterINISettings(bgsee::INISettingDepotT& Depot);
	};
#define AUXVIEWPORT							AuxiliaryViewport::GetSingleton()
}