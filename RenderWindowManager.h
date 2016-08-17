#pragma once
#include "RenderWindowGroupManager.h"
#include "PathGridUndoManager.h"
#include "RenderWindowOSD.h"
#include "RenderWindowCellLists.h"
#include <bgsee\RenderWindowFlyCamera.h>

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowManager;

		// modifes the scenegraph before rendering to the RW frame buffer
		class IRenderWindowSceneGraphModifier
		{
		public:
			virtual ~IRenderWindowSceneGraphModifier() = 0
			{
				;//
			}

			struct RenderData
			{
				NiNode*							SceneGraph;
				NiNode*							ExtraNode;			// default root node new geom can be added to
				const TESObjectREFRArrayT&		LoadedRefs;			// all the references in the current cell(grid)

				RenderData(NiNode* SceneGraph, NiNode* ExtraNode);
			};

			virtual void					PreRender(RenderData& Data) = 0;
			virtual void					PostRender(RenderData& Data) = 0;
		};

		class RenderWindowSceneGraphManager
		{
			friend class RenderWindowManager;
			typedef std::vector<IRenderWindowSceneGraphModifier*>		ModifierArrayT;

			ModifierArrayT					Modifiers;

			void							HandleRender(NiCamera* Camera,
														 NiNode* SceneGraph,
														 NiCullingProcess* CullingProc,
														 BSRenderedTexture* RenderTarget);
		public:
			RenderWindowSceneGraphManager();
			~RenderWindowSceneGraphManager();

			void							AddModifier(IRenderWindowSceneGraphModifier* Mod);
			void							RemoveModifier(IRenderWindowSceneGraphModifier* Mod);

		};

		class ReferenceParentChildIndicator : public IRenderWindowSceneGraphModifier
		{
			NiVertexColorProperty*			VertexColor;
			NiWireframeProperty*			Wireframe;

			static bool						EnableParentIndicatorVisitor(TESObjectREFR* Ref);
		public:
			ReferenceParentChildIndicator();
			virtual ~ReferenceParentChildIndicator();

			virtual void					PreRender(RenderData& Data);
			virtual void					PostRender(RenderData& Data);

			static ReferenceParentChildIndicator			Instance;
		};

		class ReferenceVisibilityModifier : public IRenderWindowSceneGraphModifier
		{
			typedef std::vector<NiNode*>			NiNodeArrayT;

			NiNodeArrayT							CulledRefBuffer;
		public:
			ReferenceVisibilityModifier();
			virtual ~ReferenceVisibilityModifier();

			virtual void					PreRender(RenderData& Data);
			virtual void					PostRender(RenderData& Data);

			static ReferenceVisibilityModifier			Instance;
		};

		class DebugSceneGraphModifier : public IRenderWindowSceneGraphModifier
		{
			NiMaterialProperty*			MatProp;
			NiStencilProperty*			Stencil;
		public:
			DebugSceneGraphModifier();
			virtual ~DebugSceneGraphModifier();

			virtual void					PreRender(RenderData& Data);
			virtual void					PostRender(RenderData& Data);

			static DebugSceneGraphModifier			Instance;
		};

		class ReferenceVisibilityValidator
		{
		public:
			static bool							ShouldBeInvisible(TESObjectREFR* Ref);			// returns true if the reference is not supposed to be rendered to the scene
			static bool							IsCulled(TESObjectREFR* Ref);					// returns true if the ref's node is culled
		};


		class RenderWindowSelectionManager
		{
			RenderWindowGroupManager*		ReferenceGroupManager;

			bool							IsSelectable(TESObjectREFR* Ref, bool& OutRegularHandling) const;
		public:
			RenderWindowSelectionManager(RenderWindowGroupManager* GroupMan);
			~RenderWindowSelectionManager();

			void							AddToSelection(TESObjectREFR* Ref, bool AddSelectionBox) const;
			void							RemoveFromSelection(TESObjectREFR* Ref, bool RemoveSelectionBox) const;
			bool							IsSelectable(TESObjectREFR* Ref) const;
		};

		class RenderWindowFlyCameraOperator : public bgsee::RenderWindowFlyCameraOperator
		{
			NiFrustum								ViewportFrustumBuffer;
		public:
			RenderWindowFlyCameraOperator(HWND ParentWindow, bgsee::ResourceTemplateT TemplateID);
			virtual ~RenderWindowFlyCameraOperator();

			virtual void							PrologCallback(void);
			virtual void							EpilogCallback(void);

			virtual void							Rotate(int XOffset, int YOffset);
			virtual void							Move(UInt8 Direction, bool Sprinting, bool Crawling);

			virtual void							RefreshRenderWindow(void);
		};


		class RenderWindowExtendedState
		{
			bool						Initialized;
		public:
			const float					MaxLandscapeEditBrushRadius;

			bool						FreezeInactiveRefs;
			bool						UseAlternateMovementSettings;
			POINT						CurrentMouseLBDragCoordDelta;
			NiFrustum					CameraFrustumBuffer;
			TESObjectREFR*				CurrentMouseRef;
			TESPathGridPoint*			CurrentMousePathGridPoint;
			POINT						CurrentMouseCoord;
			bool						ShowInitiallyDisabledRefs;
			bool						ShowInitiallyDisabledRefChildren;
			bool						UseGrassTextureOverlay;
			NiSourceTexture*			GrassOverlayTexture;
			Vector3						StaticCameraPivot;
			bool						PaintingSelection;
			UInt8						SelectionPaintingMode;

			enum : UInt8
			{
				kSelectionPaintingMode_NotSet = 0,
				kSelectionPaintingMode_Select,
				kSelectionPaintingMode_Deselect
			};

			RenderWindowExtendedState();
			~RenderWindowExtendedState();

			void						Initialize();
			void						Deinitialize();


			static RenderWindowExtendedState		Instance;
		};
#define _RENDERWIN_XSTATE		renderWindow::RenderWindowExtendedState::Instance

		class RenderWindowManager
		{
			class GlobalEventSink : public SME::MiscGunk::IEventSink
			{
				RenderWindowManager*			Parent;
			public:
				GlobalEventSink(RenderWindowManager* Parent);

				virtual void					Handle(SME::MiscGunk::IEventData* Data);
			};

			friend class GlobalEventSink;

			static LRESULT CALLBACK						RenderWindowMenuInitSelectSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
			static LRESULT CALLBACK						RenderWindowMasterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);

			RenderWindowSceneGraphManager*				SceneGraphManager;
			RenderWindowGroupManager*					GroupManager;
			PathGridUndoManager*						PGUndoManager;
			RenderWindowSelectionManager*				SelectionManager;
			RenderWindowOSD*							OSD;
			RenderWindowCellLists*						CellLists;
			GlobalEventSink*							EventSink;
			TESObjectREFRArrayT							ActiveRefCache;

			bool										Initialized;

			void										HandleD3DRelease();
			void										HandleD3DRenew();
			void										HandlePreSceneGraphRender(NiCamera* Camera,
																				  NiNode* SceneGraph,
																				  NiCullingProcess* CullingProc,
																				  BSRenderedTexture* RenderTarget);
			void										HandlePostSceneGraphRender();
			void										CacheActiveRefs();
			
			bool										RenderModalNewRefGroup(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			void										CreateRefGroup();
			void										DeleteRefGroup();
		public:
			RenderWindowManager();
			~RenderWindowManager();

			bool										Initialize();			// called before the render window is created
			bool										InitializeOSD();		// separate as the renderer is only initialized after the main windows are created
			void										Deinitialize();

			RenderWindowGroupManager*					GetReferenceGroupManager() const;
			PathGridUndoManager*						GetPathGridUndoManager() const;
			RenderWindowSelectionManager*				GetSelectionManager() const;

			void										InvokeContextMenuTool(int Identifier);
			void										RefreshFOV();
			const TESObjectREFRArrayT&					GetActiveRefs() const;

			static RenderWindowManager					Instance;
		};

#define _RENDERWIN_MGR				renderWindow::RenderWindowManager::Instance



		void Initialize(void);
		void Deinitialize(void);
	}
}