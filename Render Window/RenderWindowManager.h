#pragma once
#include "RenderWindowGroups.h"
#include "PathGridUndoManager.h"
#include "RenderWindowOSD.h"
#include "RenderWindowCellLists.h"
#include "RenderWindowInput.h"
#include "RenderWindowLayers.h"

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowManager;

		// modifes the scenegraph before rendering to the RW frame buffer
		class IRenderWindowSceneGraphModifier
		{
		public:
			virtual ~IRenderWindowSceneGraphModifier() = 0 {}

			struct RenderData
			{
				NiNode*							SceneGraph;
				NiNode*							ExtraNode;			// default root node new geom can be added to
				const TESObjectREFRArrayT&		LoadedRefs;			// all the references in the current cell(grid), including temporaries

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
			NiVertexColorProperty*		VertexColor;
			NiWireframeProperty*		Wireframe;
			bool						Initialized;

			static bool					EnableParentIndicatorVisitor(TESObjectREFR* Ref);
			void						LazyInitialize();
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

		class ReferenceVisibilityManager
		{
		public:
			// reasons why the ref ought to be invisible
			enum
			{
				kReason_InitiallyDisabledSelf		= 1 << 1,		// ref has the "Initially Disabled" flag
				kReason_InitiallyDisabledChild		= 1 << 2,		// ref is a child of a parent that is initially disabled
				kReason_InvisibleSelf				= 1 << 3,		// ref has the "Invisible" flag
				kReason_InvisibleChild				= 1 << 4,		// ref is a child of a parent that has the "Invisible Children" flag
				kReason_ParentLayerInvisible		= 1 << 5,		// ref's parent layer is invisible
			};

			static bool							ShouldBeInvisible(TESObjectREFR* Ref, UInt32& OutReasonFlags);			// returns true if the reference is not supposed to be rendered to the scene
			static bool							ShouldBeInvisible(TESObjectREFR* Ref);
			static bool							IsCulled(TESObjectREFR* Ref);											// returns true if the ref's node is culled
		};


		class ReferenceSelectionManager
		{
			static bool							IsSelectable(TESObjectREFR* Ref, bool PaintingSelection, bool& OutRegularHandling, UInt32& OutReasonFlags);
		public:
			// reasons why the ref can(not) be selected
			enum
			{
				kReason_Override				= 1 << 0,		// selection override key (ALT) is held down, allowing selection
				kReason_InvalidVisibility		= 1 << 1,		// ref is invisible
				kReason_FrozenSelf				= 1 << 2,		// ref has the "Frozen" flag
				kReason_FrozenInactive			= 1 << 3,		// ref is frozen by the "Freeze Inactive Refs" tool
				kReason_ParentLayerFrozen		= 1 << 4,		// ref's parent layer is frozen
			};

			static void							AddToSelection(TESObjectREFR* Ref, bool AddSelectionBox, bool PaintingSelection = false);
			static void							RemoveFromSelection(TESObjectREFR* Ref, bool RemoveSelectionBox);
			static bool							IsSelectable(TESObjectREFR* Ref, bool PaintingSelection = false);
			static bool							IsSelectable(TESObjectREFR* Ref, UInt32& OutReasonFlags, bool PaintingSelection = false);
			static void							RemoveInvalidEntries();
		};

		class ReferenceColorMaskManager
		{
		public:
			enum : int
			{
				kMask__BEGIN = -1,

				kMask_Selection = 0,
				kMask_MouseOver,

				kMask__MAX
			};
		private:
			struct MaskData
			{
				bool		Enabled = false;
				NiColor		Color;

				SME::INI::INISetting*
							ToggleSetting = nullptr;
				SME::INI::INISetting*
							ColorSetting = nullptr;
			};

			std::array<MaskData, kMask__MAX> Masks;
		public:
			ReferenceColorMaskManager();

			void			Initialize();
			void			Deinitialize();

			bool			IsAnyMaskEnabled() const;
			bool			GetMaskEnabled(UInt8 Mask) const;
			void			SetMaskEnabled(UInt8 Mask, bool Enabled);

			const NiColor&	GetMaskColor(UInt8 Mask) const;
			void			SetMaskColor(UInt8 Mask, const NiColor& Color);

			bool			GetActiveMaskForRef(TESObjectREFR* Ref, NiColor* OutColor) const;	// returns false if no mask is active
		};

		class RenderWindowExtendedState
		{
			bool						Initialized;
		public:
			static const float			kMaxLandscapeEditBrushRadius;

			bool						FreezeInactiveRefs;
			bool						UseAlternateMovementSettings;
			NiFrustum					CameraFrustumBuffer;
			TESObjectREFR*				CurrentMouseRef;
			TESPathGridPoint*			CurrentMousePathGridPoint;
			TESObjectCELL*				CurrentMouseExteriorCell;
			Vector3						CurrentMouseWorldIntersection;
			bool						ShowInitiallyDisabledRefs;
			bool						ShowInitiallyDisabledRefChildren;
			bool						UseGrassTextureOverlay;
			NiSourceTexture*			GrassOverlayTexture;
			Vector3						StaticCameraPivot;
			bool						DraggingPathGridPoints;
			TESObjectSTAT*				MeasureBaseRuler;
			TESObjectSTAT*				MeasureBaseCircle;

			RenderWindowExtendedState();
			~RenderWindowExtendedState();

			void						Initialize();
			void						Deinitialize();

			const Vector3&				UpdateStaticCameraPivot();
		};

		// executes (and consumes) tasks that are unsafe to be performed elsewhere
		class RenderWindowDeferredExecutor
		{
		public:
			typedef std::function<void()>		DelegateT;
		private:
			friend class RenderWindowManager;

			typedef std::vector<DelegateT>		DelegateArrayT;

			DelegateArrayT	Handlers;
			bool			Executing = false;

			void			HandlePostRenderWindowUpdate();
		public:
			void			QueueTask(DelegateT Delegate);		// consumes the task after execution
		};

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

			static LRESULT CALLBACK						RenderWindowMenuInitSelectSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																							bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
			static LRESULT CALLBACK						RenderWindowMasterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																					bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);

			RenderWindowExtendedState*					ExtendedState;
			RenderWindowSceneGraphManager*				SceneGraphManager;
			RenderWindowGroupManager*					GroupManager;
			PathGridUndoManager*						PGUndoManager;
			RenderWindowOSD*							OSD;
			RenderWindowCellLists*						CellLists;
			RenderWindowLayerManager*					LayerManager;
			input::RenderWindowKeyboardManager*			KeyboardInputManager;
			input::RenderWindowMouseManager*			MouseInputManager;
			RenderWindowDeferredExecutor*				DeferredExecutor;
			ReferenceColorMaskManager*					ColorMaskManager;
			GlobalEventSink*							EventSink;
			TESObjectREFRArrayT							ActiveRefCache;		// include temporary refs
			bool										RenderingScene;
			bool										Initialized;
			bool										MouseInClientArea;

			void										HandleClearData();
			void										HandleConstructSpecialForms();
			void										HandleD3DRelease();
			void										HandleD3DRenew();
			void										HandlePreSceneGraphRender(NiCamera* Camera,
																				  NiNode* SceneGraph,
																				  NiCullingProcess* CullingProc,
																				  BSRenderedTexture* RenderTarget);
			void										HandlePostSceneGraphRender();
			void										HandlePostRenderWindowUpdate();
			void										CacheActiveRefs();

			enum
			{
				kActiveRefCollection_Invisible = 0,
				kActiveRefCollection_Frozen,
				kActiveRefCollection_Groups,

				kActiveRefCollection__MAX
			};

			struct EditGroupMembersData
			{
				std::string					GroupName;
				TESObjectREFRArrayT			MemberList;
			};

			void										RenderActiveRefCollectionRefTable(UInt8 Type, ImGuiTextFilter& FilterHelper);
			void										RenderActiveRefCollectionButton(UInt8 Type);
			void										RenderActiveRefCollectionPopup(UInt8 Type);
			static bool									RenderModalEditGroupMembers(RenderWindowOSD* OSD, ImGuiDX9* GUI, void* UserData);
		public:
			RenderWindowManager();
			~RenderWindowManager();

			bool										Initialize();			// called before the render window is created
			bool										InitializeOSD();		// separate as the renderer is only initialized after the main windows are created
			void										Deinitialize();

			RenderWindowGroupManager*					GetGroupManager() const;
			PathGridUndoManager*						GetPathGridUndoManager() const;
			RenderWindowExtendedState&					GetState() const;
			input::RenderWindowKeyboardManager*			GetKeyboardInputManager() const;
			input::RenderWindowMouseManager*			GetMouseInputManager() const;
			RenderWindowOSD*							GetOSD() const;
			RenderWindowLayerManager*					GetLayerManager() const;
			RenderWindowDeferredExecutor*				GetDeferredExecutor() const;
			ReferenceColorMaskManager*					GetColorMaskManager() const;

			const TESObjectREFRArrayT&					GetActiveRefs() const;

			void										RefreshFOV();
			bool										IsRenderingScene() const;

			static RenderWindowManager					Instance;
		};

#define _RENDERWIN_MGR				renderWindow::RenderWindowManager::Instance
#define _RENDERWIN_XSTATE			_RENDERWIN_MGR.GetState()


		void Initialize(void);
		void Deinitialize(void);
	}
}