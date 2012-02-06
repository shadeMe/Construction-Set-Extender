#pragma  once

class AuxiliaryViewport
{
	static AuxiliaryViewport*			Singleton;

	HWND								WindowHandle;
	bool								Valid;
	bool								DisplayState;
	NiCamera*							ViewportCamera;
	bool								Frozen;

	AuxiliaryViewport();

	void								LoadINISettings();
	void								SaveINISettings();
public:
	static AuxiliaryViewport*			GetSingleton();

	bool								IsHidden() const { return DisplayState == 0; }
	bool								IsInitialized() const { return Valid; }
	bool								IsFrozen() const { return Frozen; }
	bool								ToggleDisplayState();
	bool								ToggleFrozenState();

	void								SyncViewportCamera(NiCamera* Camera);

	void								Draw(NiNode* NodeToRender, NiCamera* Camera);
	void								DrawBackBuffer(void);
	void								Redraw();
	void								ClearScreen();
	
	void								Initialize();
	void								Deinitialize();
};

#define AUXVIEWPORT						AuxiliaryViewport::GetSingleton()