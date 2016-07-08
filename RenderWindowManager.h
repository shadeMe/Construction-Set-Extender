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
				NiNode*				SceneGraph;
				NiNode*				ExtraNode;			// default root node new geom can be added to
				TESObjectREFRArrayT		LoadedRefs;			// all the references in the current cell(grid)

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


		class RenderWindowSelectionManager
		{
			RenderWindowGroupManager*		ReferenceGroupManager;
		public:
			RenderWindowSelectionManager(RenderWindowGroupManager* GroupMan);
			~RenderWindowSelectionManager();

			void							AddToSelection(TESObjectREFR* Ref, bool SelectionBox) const;
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



		// result = Vector3*
#define WM_RENDERWINDOW_GETCAMERASTATICPIVOT	(WM_USER + 2005)
#define WM_RENDERWINDOW_UPDATEFOV				(WM_USER + 2010)


		class RenderWindowManager
		{
			class DialogExtraData : public bgsee::WindowExtraData
			{
			public:
				bool				TunnelingKeyMessage;

				DialogExtraData();
				virtual ~DialogExtraData();

				enum { kTypeID = 'XRWM' };
			};

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

			bool										Initialized;

			void										HandleD3DRelease();
			void										HandleD3DRenew();
			void										HandleSceneGraphRender(NiCamera* Camera, NiNode* SceneGraph, NiCullingProcess* CullingProc, BSRenderedTexture* RenderTarget);
			void										HandlePostSceneGraphRender();
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

			static RenderWindowManager					Instance;
		};

#define _RENDERWIN_MGR				renderWindow::RenderWindowManager::Instance



		void Initialize(void);
		void Deinitialize(void);
	}
}