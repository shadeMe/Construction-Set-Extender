#pragma once

#include "TESPathGrid.h"
#include "TESDialog.h"

//	EditorAPI: Render control related classes.

class	TESForm;
class	TESObjectREFR;

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

// container class for the editor's renderer implementation
class TESRender
{
public:
	// arbitrarily named
	// 0C
	class PrimaryRenderer
	{
	public:
		// members
		/*00*/ NiWindow*					niWindow;
		/*04*/ NiNode*						primaryCameraParentNode;
		/*08*/ NiCamera*					primaryCamera;

		// methods
		void								RenderNode(NiCamera* Camera = NULL, NiNode* NodeToRender = NULL, BSRenderedTexture* RenderToTexture = NULL);
		void								GetCameraPivot(Vector3* OutPivot, float ScaleFactor);

		static PrimaryRenderer**			Singleton;
	};
	STATIC_ASSERT(sizeof(PrimaryRenderer) == 0x0C);

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

	// methods
	static bool								UpdateNode(NiNode* Node, UInt32 UpdateType, float Multiplier);
	static void								RotateNode(NiNode* Node, Vector3* Pivot, int XOffset, int YOffset, float SpeedMultiplier);
	static void								SetCameraFOV(NiCamera* Camera, float FOV, float Width = -1, float Height = -1);		// horizontal FOV
	static TESObjectREFR*					PickAtCoords(int X, int Y);

	// misc methods that belong elswhere
	static void								UpdateAVObject(NiAVObject* Object);		// NiAVObject method
	static NiNode*							CreateNiNode();
	static void								DeleteNiNode(NiNode* Node);
	static void								AddToNiNode(NiNode* To, NiAVObject* Child);

	static NiDX9Renderer**					NiRendererSingleton;
};

#define _NIRENDERER				(*TESRender::NiRendererSingleton)
#define _PRIMARYRENDERER		(*TESRender::PrimaryRenderer::Singleton)

typedef std::vector<TESObjectREFR*>		CellObjectListT;

// container class, arbitrarily named
class TESRenderWindow
{
public:
	enum
	{
		kRenderWindowState_SnapToGrid					= 0x1,
		kRenderWindowState_SnapToAngle					= 0x2,
		kRenderWindowState_AllowRenderWindowCellLoads	= 0x4,
		kRenderWindowState_ShowMarkers					= 0x8,
		kRenderWindowState_SkipInitialCellLoad			= 0x10,
		kRenderWindowState_UseCSDiscAsSource			= 0x20,		// deprecated, probably from the TES3 CS codebase
		kRenderWindowState_UseWorld						= 0x40,
	};

	// 08
	class UndoStack
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
			/*04*/ UInt32				selCount;						// number of refs in the selection of which this ref was a part
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
	STATIC_ASSERT(sizeof(UndoStack) == 0x08);

	// 60
	class RubberBandSelection
	{
	public:
		/*00*/ int							originXCoord;			// set at the start of a drag op, i.e., inside the WM_LBUTTONDOWN handler
		/*04*/ int							originYCoord;
		/*08*/ Vector3						unk08;
		/*14*/ Vector3						bandVertices[4];
		/*44*/ NiVector3*					vertexBuffer;			// 4x dynamic array, passed to NiLines ctor
		/*48*/ NiNode*						sceneNode;
		/*4C*/ NiCamera*					sceneCamera;
		/*50*/ NiLines*						bandBox;				// smart ptr
		/*54*/ float						unk54;					// init to 60.0f, some sort of multiplier used when updating the band box's vertices
		/*58*/ TESRenderSelection*			selectionBuffer;		// allocated by ctor
		/*5C*/ UInt8						dragging;				// set to 1 during selection operations
		/*5D*/ UInt8						hasSelection;			// set to 0 if the area of the band box is zero, i.e., if one of the current mouse coords is the same as that of the origin
		/*5E*/ UInt16						pad5E;
	};
	STATIC_ASSERT(sizeof(RubberBandSelection) == 0x60);

	// methods
	static void							Reset();
	static void							Redraw(bool RefreshPathGrid = false);
	static void							Refresh3D();
	static void							TogglePathGridEditMode();
	static UInt32						GetActiveCellObjects(CellObjectListT& OutList);		// enumerates refs in the current interior/exterior grid and returns the count

	static HWND*						WindowHandle;
	static TESRenderSelection**			ClipboardSelection;
	static UndoStack**					UndoBuffer;
	static RubberBandSelection**		RubberBandSelector;

	static UInt32*						StateFlags;

	static UInt8*						RefreshFlag;
	static UInt8*						PathGridEditFlag;
	static UInt8*						LandscapeEditFlag;

	static TESObjectCELL**				ActiveCell;						// points to the current interior cell or the exterior cell at the camera's position
	static TESLandTexture**				ActiveLandscapeTexture;

	static float*						RefMovementSpeed;
	static float*						RefRotationSpeed;
	static float*						SnapAngle;
	static float*						SnapGridDistance;
	static float*						CameraRotationSpeed;
	static float*						CameraZoomSpeed;
	static float*						CameraPanSpeed;

	static PathGridPointListT*			SelectedPathGridPoints;

	static HCURSOR*						CursorArrow;
	static HCURSOR*						CursorSelect;
	static HCURSOR*						CursorMove;

	static bool							FreezeInactiveRefs;
	static bool							UseAlternateMovementSettings;
	static POINT						CurrentMouseLBDragCoordDelta;
	static NiFrustum					CameraFrustumBuffer;
	static TESObjectREFR*				CurrentMouseRef;
	static POINT						CurrentMouseCoord;
	static bool							ShowInitiallyDisabledRefs;
	static bool							ShowInitiallyDisabledRefChildren;

	static const float					MaxLandscapeEditBrushRadius;

	enum
	{
		kTimer_ViewportUpdate = 1,
	};
};

#define _RENDERUNDO				(*TESRenderWindow::UndoBuffer)

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

// preview controls used in various dialogs
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

	enum { kPreviewOutputCtrlID = 2175 };

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
	/*40*/ int						currentMouseXCoord;
	/*44*/ int						currentMouseYCoord;

	// methods
	virtual void				Update(void) = 0;
	virtual void				Render(void) = 0;
	virtual void				Reset(void) = 0;
	virtual void				HandleDeviceReset(void) = 0;
	virtual void				Dtor(bool ReleaseMemory) = 0;
	virtual void				AddPreviewNode(NiNode* Node) = 0;			// adds the node to the scene root and increments its ref count
	virtual void				RemovePreviewNode(NiNode* Node) = 0;		// removes the node from the scene root and decrements its ref count
	virtual void				CenterCamera(void) = 0;
	virtual void				HandleResize(void) = 0;
	virtual void				Present(float Time = 0.f) = 0;
	virtual LRESULT				DialogMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lPARAM, LONG_PTR OutUnk05);		// OutUnk05 set to 1 inside the WM_DRAWITEM callback, whenin the method call Render()
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
	/*64*/ ShadowSceneNode*			previewSceneRoot;		// smart ptr, shared by all preview controls, destroyed with the last one and created with the first

	// methods
	void							RemovePreviewNode(NiNode* Node);	// this override appears to be bugged in the editor code as it calls AddPreviewNode
																		// so manually call the correct TESRenderControl method
																		// could have something to do with the vanilla ModelLoader NIF leakage
	void							RemoveGroundPlane();

	static TESPreviewControl*		CreatePreviewControl(HWND Dialog, const TESRenderControl::Parameters* Params);
	static void						UpdatePreviewWindows(bool RefreshRenderWindow = true);

	typedef tList<TESPreviewControl>	PreviewControlListT;

	static PreviewControlListT*			ActivePreviewControls;
};
STATIC_ASSERT(sizeof(TESPreviewControl) == 0x68);
