#pragma  once

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		BGSEditorExtender::BGSEEINIManagerSettingFactory*		GetAuxiliaryViewport(void);
	}

	class AuxiliaryViewport : public BGSEditorExtender::BGSEEGenericModelessDialog
	{
		friend BGSEditorExtender::BGSEEINIManagerSettingFactory* INISettings::GetAuxiliaryViewport(void);

		static AuxiliaryViewport*			Singleton;
	protected:
		static LRESULT CALLBACK				BaseDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);

		static const char*					kWindowTitle;
		static const char*					kINISection;

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
	};
#define AUXVIEWPORT							AuxiliaryViewport::GetSingleton()
}