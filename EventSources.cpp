#include "EventSources.h"

using namespace SME::MiscGunk;

namespace cse
{
	namespace events
	{

		TypedEventSource::TypedEventSource(UInt32 TypeID) :
			IEventSource(),
			TypeID(TypeID)
		{
			SME_ASSERT(TypeID > kType__BEGIN && TypeID < kType__END);
		}

		const UInt32 TypedEventSource::GetTypeID() const
		{
			return TypeID;
		}

		BasicEventSource::BasicEventSource(UInt32 Type) :
			TypedEventSource(Type)
		{
			;//
		}

		void BasicEventSource::RaiseEvent() const
		{
			IEventData Data(this);
			Dispatch(&Data);
		}

		namespace plugin
		{

			BasicTESFileEventSource::BasicTESFileEventSource(UInt32 Type) :
				TypedEventSource(Type)
			{
				;//
			}

			void BasicTESFileEventSource::RaiseEvent(TESFile* File) const
			{
				TESFileEventData Data(this, File);
				Dispatch(&Data);
			}

			TESFileEventData::TESFileEventData(const BasicTESFileEventSource* Source, TESFile* File) :
				IEventData(Source),
				File(File)
			{
				;//
			}

			BasicEventSource					kPreLoad(TypedEventSource::kType_Plugin_PreLoad);
			BasicEventSource					kPostLoad(TypedEventSource::kType_Plugin_PostLoad);
			BasicTESFileEventSource				kPreSave(TypedEventSource::kType_Plugin_PreSave);
			BasicEventSource					kPostSave(TypedEventSource::kType_Plugin_PostSave);
			BasicEventSource					kClearData(TypedEventSource::kType_Plugin_ClearData);
		}

		namespace renderer
		{
			PreSceneGraphRenderData::PreSceneGraphRenderData(const PreSceneGraphRenderEventSource* Source, NiCamera* Camera, NiNode* SceneGraph, NiCullingProcess* CullingProc, BSRenderedTexture* RenderTarget) :
				IEventData(Source),
				Camera(Camera),
				SceneGraph(SceneGraph),
				CullingProc(CullingProc),
				RenderTarget(RenderTarget)
			{
				;//
			}

			PreSceneGraphRenderEventSource::PreSceneGraphRenderEventSource() :
				TypedEventSource(kType_Renderer_PreMainSceneGraphRender)
			{

			}

			void PreSceneGraphRenderEventSource::RaiseEvent(NiCamera* Camera, NiNode* SceneGraph, NiCullingProcess* CullingProc, BSRenderedTexture* RenderTarget) const
			{
				PreSceneGraphRenderData Data(this, Camera, SceneGraph, CullingProc, RenderTarget);
				Dispatch(&Data);
			}

			BasicEventSource						kRelease(TypedEventSource::kType_Renderer_Release);
			BasicEventSource						kRenew(TypedEventSource::kType_Renderer_Renew);
			PreSceneGraphRenderEventSource			kPreSceneGraphRender;
			BasicEventSource						kPostSceneGraphRender(TypedEventSource::kType_Renderer_PostMainSceneGraphRender);
		}

		namespace dialog
		{
			BasicEventSource						kCloseAll(TypedEventSource::kType_Dialog_CloseAll);
		}

		namespace form
		{

			TESFormEventData::TESFormEventData(const BasicTESFormEventSource* Source, TESForm* Form, UInt8 Type) :
				IEventData(Source),
				Form(Form),
				EventType(Type)
			{
				NewEditorID = NULL;
				NewFormID = 0;
				ActiveState = false;
				DeletedState = false;
			}

			BasicTESFormEventSource::BasicTESFormEventSource(UInt32 Type) :
				TypedEventSource(Type)
			{

			}

			void BasicTESFormEventSource::HandleInstantiation(TESForm* Form) const
			{
				TESFormEventData Data(this, Form, TESFormEventData::kType_Instantiation);
				Dispatch(&Data);
			}

			void BasicTESFormEventSource::HandleSetActive(TESForm* Form, bool State) const
			{
				TESFormEventData Data(this, Form, TESFormEventData::kType_SetActive);
				Data.ActiveState = State;
				Dispatch(&Data);
			}

			void BasicTESFormEventSource::HandleSetDeleted(TESForm* Form, bool State) const
			{
				TESFormEventData Data(this, Form, TESFormEventData::kType_SetDeleted);
				Data.DeletedState = State;
				Dispatch(&Data);
			}

			void BasicTESFormEventSource::HandleSetFormID(TESForm* Form, UInt32 FormID) const
			{
				TESFormEventData Data(this, Form, TESFormEventData::kType_SetFormID);
				Data.NewFormID = FormID;
				Dispatch(&Data);
			}

			void BasicTESFormEventSource::HandleSetEditorID(TESForm* Form, const char* EditorID) const
			{
				TESFormEventData Data(this, Form, TESFormEventData::kType_SetEditorID);
				Data.NewEditorID = EditorID;
				Dispatch(&Data);
			}



			BasicTESFormEventSource			kInstantiation(TypedEventSource::kType_Form_Instantiation);
			BasicTESFormEventSource			kSetActive(TypedEventSource::kType_Form_SetActive);
			BasicTESFormEventSource			kSetDeleted(TypedEventSource::kType_Form_SetDeleted);
			BasicTESFormEventSource			kSetFormID(TypedEventSource::kType_Form_SetFormID);
			BasicTESFormEventSource			kSetEditorID(TypedEventSource::kType_Form_SetEditorID);
		}

		void InitializeSources()
		{
			;//
		}

		void DeinitializeSources()
		{
			plugin::kPreLoad.ClearSinks();
			plugin::kPostLoad.ClearSinks();
			plugin::kPreSave.ClearSinks();
			plugin::kPostSave.ClearSinks();
			plugin::kClearData.ClearSinks();

			renderer::kRelease.ClearSinks();
			renderer::kRenew.ClearSinks();
			renderer::kPreSceneGraphRender.ClearSinks();
			renderer::kPostSceneGraphRender.ClearSinks();

			dialog::kCloseAll.ClearSinks();

			form::kInstantiation.ClearSinks();
			form::kSetActive.ClearSinks();
			form::kSetDeleted.ClearSinks();
			form::kSetFormID.ClearSinks();
			form::kSetEditorID.ClearSinks();
		}


	}
}