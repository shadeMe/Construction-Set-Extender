#pragma once

// quite clunky and can be done better with templates
// if only I had thought of that earlier...
namespace cse
{
	namespace events
	{
		class TypedEventSource : public SME::MiscGunk::IEventSource
		{
			typedef std::vector<TypedEventSource*>		EventSourceArrayT;

			static EventSourceArrayT&	GetRegistry();
			static void					Unregister(TypedEventSource* Source);
			static void					Register(TypedEventSource* Source);
		protected:
			UInt32					TypeID;
		public:
			TypedEventSource(UInt32 TypeID);
			virtual ~TypedEventSource();

			const UInt32			GetTypeID() const;

			static void				Deinitialize();

			enum
			{
				kType__BEGIN  = 0,

				kType_Shutdown,										// when the editor shuts down

				kType_Plugin_PreLoad,								// before a load plugins op
				kType_Plugin_PostLoad,								// after a load plugins opp
				kType_Plugin_PreSave,								// right before the save file's header is updated
				kType_Plugin_PostSave,								// after the post-save cleanup is performed
				kType_Plugin_ClearData,								// when the data handler clears its data

				kType_Renderer_Release,								// before the renderer is released
				kType_Renderer_Renew,								// after the renderer is recreated/renewed
				kType_Renderer_PreMainSceneGraphRender,				// before the main scenegraph is rendered
				kType_Renderer_PostMainSceneGraphRender,			// after the main scenegraph is rendered
				kType_Renderer_PostRenderWindowUpdate,				// after the render window viewport is refreshed (post-scene rendering)
				kType_Renderer_PreBSFadeNodeDraw,					// before a BSFadeNode is accumulated for rendering
				kType_Renderer_PostBSFadeNodeDraw,					// after a BSFadeNode is accumulated for rendering

				kType_Form_Instantiation,							// when a TESForm object is instantiated
				kType_Form_SetActive,
				kType_Form_SetDeleted,
				kType_Form_SetFormID,
				kType_Form_SetEditorID,

				kType_Dialog_CloseAll,								// when the editor's child dialogs are being closed

				kType__BEGIN_CELLVIEW,

				kType_CellView_SelectCell,							// when a cell is selected in the cell list

				kType__END_CELLVIEW,

				kType__BEGIN_RENDERWINDOW,

				kType_RenderWindow_PlaceRef,						// when references are dropped into the render window from either the object/prefab/palette windows

				kType__END_RENDERWINDOW,

				kType__END
			};
		};

		class BasicEventSource : public TypedEventSource
		{
			BasicEventSource();
		public:
			BasicEventSource(UInt32 Type);

			void				RaiseEvent() const;
		};

		class BasicTESFileEventSource : public TypedEventSource
		{
			BasicTESFileEventSource();
		public:
			BasicTESFileEventSource(UInt32 Type);

			void				RaiseEvent(TESFile* File) const;
		};

		struct TESFileEventData : public SME::MiscGunk::IEventData
		{
			TESFile*			File;

			TESFileEventData(const BasicTESFileEventSource* Source, TESFile* File);
		};

		class BasicTESFormEventSource : public TypedEventSource
		{
			BasicTESFormEventSource();
		public:
			BasicTESFormEventSource(UInt32 Type);

			void						HandleInstantiation(TESForm* Form) const;
			void						HandleSetActive(TESForm* Form, bool State) const;
			void						HandleSetDeleted(TESForm* Form, bool State) const;
			void						HandleSetFormID(TESForm* Form, UInt32 FormID) const;
			void						HandleSetEditorID(TESForm* Form, const char* EditorID) const;
		};

		struct TESFormEventData : public SME::MiscGunk::IEventData
		{
			enum
			{
				kType_Instantiation = 0,
				kType_SetActive,
				kType_SetDeleted,
				kType_SetFormID,
				kType_SetEditorID
			};

			TESForm*			Form;
			UInt8				EventType;
			union
			{
				bool			ActiveState;
				bool			DeletedState;
				UInt32			NewFormID;
				const char*		NewEditorID;
			};

			TESFormEventData(const BasicTESFormEventSource* Source, TESForm* Form, UInt8 Type);
		};

		namespace general
		{
			extern BasicEventSource					kShutdown;
		}

		namespace plugin
		{
			extern BasicEventSource					kPreLoad;
			extern BasicEventSource					kPostLoad;
			extern BasicTESFileEventSource			kPreSave;
			extern BasicEventSource					kPostSave;
			extern BasicEventSource					kClearData;
		}

		namespace renderer
		{
			class PreSceneGraphRenderEventSource : public TypedEventSource
			{
			public:
				PreSceneGraphRenderEventSource();

				void				RaiseEvent(NiCamera* Camera,
											   NiNode* SceneGraph,
											   NiCullingProcess* CullingProc,
											   BSRenderedTexture* RenderTarget) const;
			};

			struct PreSceneGraphRenderData : public SME::MiscGunk::IEventData
			{
				NiCamera*			Camera;
				NiNode*				SceneGraph;
				NiCullingProcess*	CullingProc;
				BSRenderedTexture*	RenderTarget;

				PreSceneGraphRenderData(const PreSceneGraphRenderEventSource* Source,
										NiCamera* Camera, NiNode* SceneGraph, NiCullingProcess* CullingProc, BSRenderedTexture* RenderTarget);
			};

			class BSFadeNodeDrawEventSource;

			struct BSFadeNodeDrawData : public SME::MiscGunk::IEventData
			{
				enum
				{
					kType_PreDraw = 0,
					kType_PostDraw
				};

				BSFadeNode*			Node;
				TESObjectREFR*		ParentRef;
				UInt32				EventType;

				BSFadeNodeDrawData(const BSFadeNodeDrawEventSource* Source, BSFadeNode* Node, UInt32 EventType);
			};


			class BSFadeNodeDrawEventSource : public TypedEventSource
			{
			public:
				BSFadeNodeDrawEventSource(UInt32 EventType);

				void				RaiseEvent(BSFadeNode* Node) const;
			};


			extern BasicEventSource						kRelease;
			extern BasicEventSource						kRenew;
			extern PreSceneGraphRenderEventSource		kPreSceneGraphRender;
			extern BasicEventSource						kPostSceneGraphRender;
			extern BasicEventSource						kPostRenderWindowUpdate;
			extern BSFadeNodeDrawEventSource			kPreBSFadeNodeDraw;
			extern BSFadeNodeDrawEventSource			kPostBSFadeNodeDraw;
		}

		namespace dialog
		{
			extern BasicEventSource						kCloseAll;

			namespace cellView
			{
				class CellViewDialogEventSource : public TypedEventSource
				{
					CellViewDialogEventSource();
				public:
					CellViewDialogEventSource(UInt32 Type);

					void				HandleSelectCell(TESObjectCELL* Cell) const;
				};

				struct CellViewDialogEventData : public SME::MiscGunk::IEventData
				{
					enum
					{
						kType_SelectCell = 0,
					};

					TESObjectCELL*			Cell;
					UInt32					EventType;

					CellViewDialogEventData(const CellViewDialogEventSource* Source, TESObjectCELL* Cell, UInt32 Type);
				};

				extern CellViewDialogEventSource		kSelectCell;
			}

			namespace renderWindow
			{
				class RenderWindowDialogEventSource : public TypedEventSource
				{
					RenderWindowDialogEventSource();
				public:
					RenderWindowDialogEventSource(UInt32 Type);

					void				HandlePlaceRef(TESObjectREFR* NewRef) const;
				};

				struct RenderWindowDialogEventData : public SME::MiscGunk::IEventData
				{
					enum
					{
						kType_PlaceRef = 0,
					};

					UInt32					EventType;
					TESObjectREFR*			PlacedRef;

					RenderWindowDialogEventData(const RenderWindowDialogEventSource* Source, UInt32 Type);
				};

				extern RenderWindowDialogEventSource		kPlaceRef;
			}
		}

		namespace form
		{
			extern BasicTESFormEventSource			kInstantiation;
			extern BasicTESFormEventSource			kSetActive;
			extern BasicTESFormEventSource			kSetDeleted;
			extern BasicTESFormEventSource			kSetFormID;
			extern BasicTESFormEventSource			kSetEditorID;
		}


		void InitializeSources();
		void DeinitializeSources();
	}

}