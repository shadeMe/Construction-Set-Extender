#pragma  once

namespace ConstructionSetExtender
{
	class AuxiliaryViewport : public BGSEditorExtender::BGSEEGenericModelessDialog
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

		NiCamera*							ViewportCamera;
		bool								Frozen;

		AuxiliaryViewport();
	public:
		virtual ~AuxiliaryViewport();

		static AuxiliaryViewport*			GetSingleton();
		void								Initialize();

		bool								GetFrozen() const;
		bool								ToggleFrozenState();

		void								SyncViewportCamera(NiCamera* Camera);

		void								Draw(NiNode* NodeToRender, NiCamera* Camera);
		void								DrawBackBuffer(void);
		void								Redraw();
		void								ClearScreen();

		static void							RegisterINISettings(BGSEditorExtender::INISettingDepotT& Depot);
	};
#define AUXVIEWPORT							AuxiliaryViewport::GetSingleton()
}