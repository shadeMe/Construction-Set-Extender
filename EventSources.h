#pragma once


namespace cse
{
	namespace events
	{
		class TypedEventSource : public SME::MiscGunk::IEventSource
		{
		protected:
			UInt32					TypeID;
		public:
			TypedEventSource(UInt32 TypeID);

			const UInt32			GetTypeID() const;

			enum
			{
				kType__BEGIN  = 0,

				kType_Plugin_PreLoad,								// before a load plugins op
				kType_Plugin_PostLoad,								// after a load plugins opp
				kType_Plugin_PreSave,								// right before the save file's header is updated
				kType_Plugin_PostSave,								// after the post-save cleanup is performed
				kType_Plugin_ClearData,								// when the data handler clears its data

				kType_Renderer_Release,								// before the renderer is released
				kType_Renderer_Renew,								// after the renderer is recreated/renewed
				kType_Renderer_PreMainSceneGraphRender,				// before the render window's viewport is rendered
				kType_Renderer_PostMainSceneGraphRender,			// after the render window's viewport is rendered

				kType_Dialog_CloseAll,								// when the editor's child dialogs are being closed

				kType_Form_Instantiation,							// when a TESForm object is instantiated
				kType_Form_SetActive,
				kType_Form_SetDeleted,
				kType_Form_SetFormID,
				kType_Form_SetEditorID,

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

		namespace plugin
		{
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

			extern BasicEventSource						kRelease;
			extern BasicEventSource						kRenew;
			extern PreSceneGraphRenderEventSource		kPreSceneGraphRender;
			extern BasicEventSource						kPostSceneGraphRender;
		}

		namespace dialog
		{
			extern BasicEventSource			kCloseAll;
		}

		namespace form
		{
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