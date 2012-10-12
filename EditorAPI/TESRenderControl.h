#pragma once

//	EditorAPI: Render control related classes.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

class	TESForm;
class	TESObjectREFR;
class	NiWindow;

// also used to record selections made in the object/formIdListView windows
// 18
class TESRenderSelection
{
public:
	// 0C
	struct SelectedObjectsEntry
	{
		/*00*/ TESForm*					Data;
		/*08*/ SelectedObjectsEntry*	Prev;
		/*0C*/ SelectedObjectsEntry*	Next;
	};

	// members
	/*00*/ SelectedObjectsEntry*		selectionList;
	/*04*/ UInt32						selectionCount;
	/*08*/ Vector3						selectionPositionVectorSum;
	/*14*/ float						selectionBounds;						// init to 0.0

	// methods
	void								AddToSelection(TESForm* Form, bool AddSelectionBox = false);
	void								RemoveFromSelection(TESForm* Form, bool RemoveSelectionBox = false);
	void								ClearSelection(bool RemoveSelectionBox = false);
	void								CalculatePositionVectorSum(void);
	bool								HasObject(TESForm* Form);

	static TESRenderSelection*			CreateInstance(TESRenderSelection* Source = NULL);
	void								DeleteInstance();
};
STATIC_ASSERT(sizeof(TESRenderSelection) == 0x18);

extern TESRenderSelection**		g_TESRenderSelectionPrimary;
#define _RENDERSEL				(*g_TESRenderSelectionPrimary)

// 08
class TESRenderUndoStack
{
public:
	enum
	{
		kUndoOperation_Unk01	=	1,		// used to record ref creation?
		kUndoOperation_Unk02,				// used to record ref deletion?
		kUndoOperation_Unk03,				// used to record ref 3D data
		kUndoOperation_Unk04,				// 4-6 used to record landscape changes
		kUndoOperation_Unk05,
		kUndoOperation_Unk06
	};

	// 50
	struct UndoData
	{
		/*00*/ UInt32				selIndex;						// index of the ref in its parent selection
		/*04*/ UInt32				selCount;						// number of refs in the selection this ref was a part of
		/*08*/ TESObjectREFR*		refr;
		/*0C*/ UInt32				operationType;
		/*10*/ Vector3				rotation;
		/*1C*/ Vector3				position;
		/*28*/ float				scale;
		/*2C*/ UInt32				unk2C[(0x48 - 0x2C) >> 2];		// landscape change related
		/*48*/ UndoData*			previous;
		/*4C*/ UndoData*			next;
	};

	// members
	/*00*/ UndoData*				unk00;							// initialized in c'tor, used as a bookend
	/*04*/ UndoData*				first;

	// methods
	void							RecordReference(UInt32 Operation, TESRenderSelection::SelectedObjectsEntry* Selection);
	void							Clear(void);
};
STATIC_ASSERT(sizeof(TESRenderUndoStack) == 0x08);

extern TESRenderUndoStack**		g_TESRenderUndoStack;
#define _RENDERUNDO				(*g_TESRenderUndoStack)

// 4
class Renderer
{
public:
	// members
	//*00*/ void**					vtbl;
	 
	// abstract base class
	virtual void					Render(NiCamera* Camera = NULL, NiNode* NodeToRender = NULL, BSRenderedTexture* RenderToTexture = NULL) = 0;
};
STATIC_ASSERT(sizeof(Renderer) == 0x04);

// 14
class NiWindow : public Renderer
{
public:
	typedef tList<NiPointer<NiCamera>>		RenderCameraListT;

	// members
	//     /*00*/ Renderer
	/*04*/ NiNode*					sceneRootNode;			// smart pointer
	/*08*/ UInt32					renderedFrameCount;		// incremented in Render()
	/*0C*/ RenderCameraListT		attachedCameras;

	// methods
	virtual void					SetSceneNode(NiNode* Node) = 0;
	virtual NiCamera*				AddCamera(NiCamera** OutCamera) = 0;
	virtual bool					HandleResize(void) = 0;
	virtual bool					UpdateCameraFrustum(NiCamera* Camera, int Width, int Height, float ClipDistance) = 0;
};
STATIC_ASSERT(sizeof(NiWindow) == 0x14);

// 0C
class TESRenderComponents
{
public:
	enum
	{
		kNodeUpdate_Unk00 =		0,		// translate X axis?
		kNodeUpdate_Unk01,				// translate Y axis?
		kNodeUpdate_Unk02,				// translate Z axis?
		kNodeUpdate_Unk03,
		kNodeUpdate_Unk04,				// zoom?
		kNodeUpdate_Unk05,
		kNodeUpdate_Unk06,
		kNodeUpdate_Unk07,
		kNodeUpdate_Unk08,
		kNodeUpdate_Unk09,
		kNodeUpdate_Unk010,
		kNodeUpdate_Unk011,
	};

	// members
	/*00*/ NiWindow*					niWindow;
	/*04*/ NiNode*						primaryCameraParentNode;
	/*08*/ NiCamera*					primaryCamera;

	// methods
	void								RenderNode(NiCamera* Camera = NULL, NiNode* NodeToRender = NULL, BSRenderedTexture* RenderToTexture = NULL);
	void								GetCameraPivot(Vector3* OutPivot, float ScaleFactor);
	bool								UpdateNode(NiNode* Node, UInt32 UpdateType, float Multiplier);
	void								RotateNode(NiNode* Node, Vector3* Pivot, int XOffset, int YOffset, float SpeedMultiplier);
	void								SetCameraFOV(NiCamera* Camera, float FOV, float Width = -1, float Height = -1);

	// this should be moved to a better location
	void								UpdateAVObject(NiAVObject* Object);
};
STATIC_ASSERT(sizeof(TESRenderComponents) == 0x0C);

extern TESRenderComponents**	g_TESRenderComponents;
#define _RENDERCMPT				(*g_TESRenderComponents)

// arbitrarily named
// used in the runtime as well, IIRC
// 20
class TESSceneNodeDebugData
{
public:
	// members
	/*00*/ HINSTANCE				instance;
	/*04*/ HWND						parent;
	/*08*/ HWND						debugWindow;
	/*0C*/ HWND						treeView;
	/*10*/ HTREEITEM				rootTreeItem;
	/*14*/ HIMAGELIST				imageList;
	/*18*/ NiNode*					sceneNode;
	/*1C*/ char*					windowTitle;		// allocated by the ctor

	// methods
	TESSceneNodeDebugData*			Initialize(HINSTANCE Instance, HWND Parent, NiNode* Node, const char* WindowTitle, int X, int Y, int Width, int Height);
};
STATIC_ASSERT(sizeof(TESSceneNodeDebugData) == 0x20);

// 48
class TESRenderControl
{
public:
	// passed to the ctor
	// 0C
	struct Parameters
	{
		/*00*/ int				previewOutputCtrlID;		// button
		/*04*/ float			renderTargetWidth;
		/*08*/ float			renderTargetHeight;
	};

	// members
	//*00*/ void**					vtbl;
	/*04*/ int						outputCtrlID;
	/*08*/ float					renderTargetWidth;
	/*0C*/ float					renderTargetHeight;
	/*10*/ HWND						parentDialog;
	/*14*/ NiNode*					sceneRoot;		// smart ptr
	/*18*/ NiDirectionalLight*		sceneLight;		// smart ptr
	/*1C*/ NiNode*					cameraNode;		// smart ptr
	/*20*/ NiCamera*				camera;			// smart ptr
	/*24*/ NiWindow*				renderer;
	/*28*/ UInt32					unk28;
	/*2C*/ UInt32					unk2C;
	/*30*/ UInt32					unk30;
	/*34*/ UInt32					unk34;
	/*38*/ UInt32					unk38;
	/*3C*/ TESSceneNodeDebugData*	debugData;
	/*40*/ UInt32					currentMouseXCoord;
	/*44*/ UInt32					currentMouseYCoord;


	// methods
	virtual void				Update(void) = 0;
	virtual void				Render(void) = 0;
	virtual void				Reset(void) = 0;
	virtual void				HandleDeviceReset(void) = 0;
	virtual void				Dtor(bool ReleaseMemory) = 0;
	virtual void				SetPreviewNode(NiNode* Node) = 0;
	virtual void				VFn18(void* Unk01) = 0;
	virtual void				CenterCamera(void) = 0;
	virtual void				HandleResize(void) = 0;
	virtual void				Present(float Time) = 0;
	virtual LRESULT				DialogMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lPARAM, LONG_PTR Unk05);		// Unk05 set to 1 by the dialog msg callback calls Render()
};
STATIC_ASSERT(sizeof(TESRenderControl) == 0x48);

// 68
class TESPreviewControl : public TESRenderControl
{
public:
	// members
	//     /*00*/ TESRenderControl
	/*48*/ UInt8					timerEnabled;			// set to 1 when the preview control is primed to handle WM_TIMER messages
	/*49*/ UInt8					pad49[3];
	/*4C*/ SInt32					elapsedTicks;			// set inside the message callback's WM_TIMER handler
	/*50*/ float					remainingTime;			// in seconds, used when previewing animations/facegen (primarily used to play a sound file after a certain amount of time), set to -1 otherwise
	/*54*/ BSString					soundFileName;			// name/path of the sound file that's played once an animation has completed its cycle
	/*5C*/ UInt8					playbackCompleted;		// set to 1 on playback completion
	/*5D*/ UInt8					pad5D[3];
	/*60*/ NiNode*					groundPlaneNode;		// smart ptr
	/*64*/ ShadowSceneNode*			previewSceneRoot;		// smart ptr, shared by all preview controls, destroyed with the last one
};
STATIC_ASSERT(sizeof(TESPreviewControl) == 0x68);

enum
{
	kRenderWindowState_SnapToGrid					= 0x1,
	kRenderWindowState_SnapToAngle					= 0x2,
	kRenderWindowState_AllowRenderWindowCellLoads	= 0x4,
	kRenderWindowState_ShowMarkers					= 0x8,
	kRenderWindowState_SkipInitialCellLoad			= 0x10,
	kRenderWindowState_UseCSDiscAsSource			= 0x20,		// deprecated
	kRenderWindowState_UseWorld						= 0x40,
};

extern UInt32*					g_RenderWindowStateFlags;
extern UInt8*					g_RenderWindowUpdateViewPortFlag;
extern TESLandTexture**			g_ActiveLandscapeEditTexture;
extern float*					g_RenderWindowRefMovementSpeed;
extern float*					g_RenderWindowSnapGridDistance;
extern float*					g_RenderWindowRefRotationSpeed;
extern float*					g_RenderWindowSnapAngle;
extern float*					g_RenderWindowCameraRotationSpeed;
extern float*					g_RenderWindowCameraZoomSpeed;
extern float*					g_RenderWindowCameraPanSpeed;

class TESPathGridPoint;
class TESPathGrid;

extern UInt8*					g_RenderWindowPathGridEditModeFlag;
extern tList<TESPathGridPoint>*	g_RenderWindowSelectedPathGridPoints;
extern TESObjectCELL**			g_RenderWindowCurrentlyLoadedCell;

extern tList<TESRenderControl>*	g_ActivePreviewControls;