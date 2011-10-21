#pragma  once

// most of the D3D code was made redundant by the 'NiRenderer Re-Present' epiphany
// left as it is for the sake of posterity

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

	/** DEPRECATED *********************
	**LPDIRECT3D9						D3DObject;
	**LPDIRECT3DDEVICE9					D3DDevice;
	**LPDIRECT3DVERTEXBUFFER9			VertexBuffer;
	**
	**
	**void								Release();
	**bool								Recreate();
	**void								RenderTextureOnDisk(const char* Path);
	**void								RenderTextureInMemory(LPDIRECT3DTEXTURE9 Texture, bool TakeOwnership, bool ReleaseAfterUse = false);
	**const LPDIRECT3DDEVICE9			GetDevice() const { return D3DDevice; }
	 **********************************/
public:
	static AuxiliaryViewport*			GetSingleton();

	bool								IsHidden() const { return DisplayState == 0; }
	bool								IsInitialized() const { return Valid; }
	bool								IsFrozen() const { return Frozen; }
	bool								ToggleDisplayState();
	bool								ToggleFrozenState();

	const HWND							GetWindow() const { return WindowHandle; }
	NiCamera*							GetViewportCamera() const { return ViewportCamera; }
	void								SyncViewportCamera(NiCamera* Camera);

	void								Redraw();
	void								ClearScreen();

	void								Initialize();
	void								Deinitialize();
};

#define AUXVIEWPORT						AuxiliaryViewport::GetSingleton()