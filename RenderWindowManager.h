#pragma once
#include "RenderWindowGroupManager.h"
#include "PathGridUndoManager.h"
#include "RenderWindowPainter.h"
#include <bgsee\RenderWindowFlyCamera.h>

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowGroupManager;
		class RenderWindowManager;
		class PathGridUndoManager;

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
				CellObjectListT		LoadedRefs;			// all the references in the current cell(grid)

				RenderData(NiNode* SceneGraph, NiNode* ExtraNode);
			};

			virtual void					PreRender(RenderData& Data) = 0;
			virtual void					PostRender(RenderData& Data) = 0;
		};

		class RenderWindowSceneGraphManager
		{
			typedef std::vector<IRenderWindowSceneGraphModifier*>		ModifierListT;

			ModifierListT					Modifiers;
		public:
			RenderWindowSceneGraphManager();
			~RenderWindowSceneGraphManager();

			void							AddModifier(IRenderWindowSceneGraphModifier* Mod);		// takes ownership of pointer
			void							HandleRender(NiCamera* Camera,
														 NiNode* SceneGraph,
														 NiCullingProcess* CullingProc,
														 BSRenderedTexture* RenderTarget);
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
		};

		class ReferenceVisibilityModifier : public IRenderWindowSceneGraphModifier
		{
			typedef std::vector<NiNode*>			NiNodeListT;

			NiNodeListT								CulledRefBuffer;
		public:
			ReferenceVisibilityModifier();
			virtual ~ReferenceVisibilityModifier();

			virtual void					PreRender(RenderData& Data);
			virtual void					PostRender(RenderData& Data);
		};


		class RenderWindowSelectionManager
		{
			RenderWindowGroupManager*		ReferenceGroupManager;
		public:
			RenderWindowSelectionManager(RenderWindowGroupManager* GroupMan);
			~RenderWindowSelectionManager();

			void							AddToSelection(TESObjectREFR* Ref, bool SelectionBox);
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
			class RenderWindowDialogExtraData : public bgsee::WindowExtraData
			{
			public:
				bool				TunnelingKeyMessage;

				RenderWindowDialogExtraData();
				virtual ~RenderWindowDialogExtraData();

				enum { kTypeID = 'XRWM' };
			};

			static LRESULT CALLBACK		RenderWindowMenuInitSelectSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
			static LRESULT CALLBACK		RenderWindowMiscSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);

			RenderWindowSceneGraphManager*				SceneGraphManager;
			RenderWindowGroupManager*					GroupManager;
			PathGridUndoManager*						PGUndoManager;
			RenderWindowSelectionManager*				SelectionManager;

			bool										Initialized;
		public:
			RenderWindowManager();
			~RenderWindowManager();


			bool										Initialize();

			RenderWindowSceneGraphManager*				GetSceneGraphManager() const;
			RenderWindowGroupManager*					GetReferenceGroupManager() const;
			PathGridUndoManager*						GetPathGridUndoManager() const;
			RenderWindowSelectionManager*				GetSelectionManager() const;

			static RenderWindowManager					Instance;
		};

#define _RENDERWIN_MGR				RenderWindowManager::Instance

		void Initialize(void);
		void Deinitialize(void);
	}
}