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
			Register(this);
		}

		const UInt32 TypedEventSource::GetTypeID() const
		{
			return TypeID;
		}

		TypedEventSource::~TypedEventSource()
		{
			DEBUG_ASSERT(Sinks.size() == 0);
			Unregister(this);
		}

		TypedEventSource::EventSourceArrayT& TypedEventSource::GetRegistry()
		{
			static EventSourceArrayT kRegistry;
			return kRegistry;
		}

		void TypedEventSource::Register(TypedEventSource* Source)
		{
			EventSourceArrayT& Registry = GetRegistry();
			EventSourceArrayT::iterator Match = std::find(Registry.begin(), Registry.end(), Source);
			DEBUG_ASSERT(Match == Registry.end());

			GetRegistry().push_back(Source);
		}

		void TypedEventSource::Unregister(TypedEventSource * Source)
		{
			EventSourceArrayT& Registry = GetRegistry();
			EventSourceArrayT::iterator Match = std::find(Registry.begin(), Registry.end(), Source);
			DEBUG_ASSERT(Match != Registry.end());

			Registry.erase(Match);
		}

		void TypedEventSource::Deinitialize()
		{
			EventSourceArrayT& Registry = GetRegistry();
			bool HasActiveSinks = false;
			for (auto Itr : Registry)
			{
				if (Itr->Sinks.size())
				{
					HasActiveSinks = true;
					BGSEECONSOLE_MESSAGE("TypedEventSource %d has %d active sinks at shutdown", (UInt32)Itr->TypeID, Itr->Sinks.size());
				}
			}

			SME_ASSERT(HasActiveSinks == false);
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

		TESFormEventData::TESFormEventData(const BasicTESFormEventSource* Source, TESForm* Form, UInt8 Type) :
			IEventData(Source),
			Form(Form),
			EventType(Type)
		{
			NewEditorID = nullptr;
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

		namespace general
		{
			BasicEventSource					kShutdown(TypedEventSource::kType_Shutdown);
		}

		namespace plugin
		{
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


			BSFadeNodeDrawData::BSFadeNodeDrawData(const BSFadeNodeDrawEventSource* Source, BSFadeNode* Node, UInt32 EventType) :
				IEventData(Source),
				Node(Node),
				EventType(EventType)
			{
				auto RefProp = NI_CAST(TESRender::GetExtraData(Node, "REF"), TESObjectExtraData);
				if (RefProp)
					ParentRef = RefProp->refr;
				else
					ParentRef = nullptr;
			}

			BSFadeNodeDrawEventSource::BSFadeNodeDrawEventSource(UInt32 EventType) :
				TypedEventSource(EventType)
			{
				;//
			}

			void BSFadeNodeDrawEventSource::RaiseEvent(BSFadeNode* Node) const
			{
				BSFadeNodeDrawData Data(this, Node, GetTypeID() == TypedEventSource::kType_Renderer_PreBSFadeNodeDraw ? BSFadeNodeDrawData::kType_PreDraw : BSFadeNodeDrawData::kType_PostDraw);
				Dispatch(&Data);
			}

			BasicEventSource						kRelease(TypedEventSource::kType_Renderer_Release);
			BasicEventSource						kRenew(TypedEventSource::kType_Renderer_Renew);
			PreSceneGraphRenderEventSource			kPreSceneGraphRender;
			BasicEventSource						kPostSceneGraphRender(TypedEventSource::kType_Renderer_PostMainSceneGraphRender);
			BasicEventSource						kPostRenderWindowUpdate(TypedEventSource::kType_Renderer_PostRenderWindowUpdate);
			BSFadeNodeDrawEventSource				kPreBSFadeNodeDraw(TypedEventSource::kType_Renderer_PreBSFadeNodeDraw);
			BSFadeNodeDrawEventSource				kPostBSFadeNodeDraw(TypedEventSource::kType_Renderer_PostBSFadeNodeDraw);
		}

		namespace dialog
		{
			namespace cellView
			{

				CellViewDialogEventSource::CellViewDialogEventSource(UInt32 Type) :
					TypedEventSource(Type)
				{
					SME_ASSERT(Type > kType__BEGIN_CELLVIEW && Type < kType__END_CELLVIEW);
				}

				void CellViewDialogEventSource::HandleSelectCell(TESObjectCELL* Cell) const
				{
					CellViewDialogEventData Data(this, Cell, CellViewDialogEventData::kType_SelectCell);
					Dispatch(&Data);
				}

				CellViewDialogEventData::CellViewDialogEventData(const CellViewDialogEventSource* Source, TESObjectCELL* Cell, UInt32 Type) :
					IEventData(Source),
					Cell(Cell),
					EventType(Type)
				{
					SME_ASSERT(Cell);
				}

				CellViewDialogEventSource			kSelectCell(TypedEventSource::kType_CellView_SelectCell);
			}

			namespace renderWindow
			{

				RenderWindowDialogEventSource::RenderWindowDialogEventSource(UInt32 Type) :
					TypedEventSource(Type)
				{
					SME_ASSERT(Type > kType__BEGIN_RENDERWINDOW && Type < kType__END_RENDERWINDOW);
				}

				void RenderWindowDialogEventSource::HandlePlaceRef(TESObjectREFR* NewRef) const
				{
					RenderWindowDialogEventData Data(this, RenderWindowDialogEventData::kType_PlaceRef);
					Data.PlacedRef = NewRef;
					Dispatch(&Data);
				}

				RenderWindowDialogEventData::RenderWindowDialogEventData(const RenderWindowDialogEventSource* Source, UInt32 Type) :
					IEventData(Source),
					EventType(Type)
				{
					PlacedRef = nullptr;
				}

				RenderWindowDialogEventSource		kPlaceRef(TypedEventSource::kType_RenderWindow_PlaceRef);
			}

			BasicEventSource						kCloseAll(TypedEventSource::kType_Dialog_CloseAll);
		}

		namespace form
		{
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
			TypedEventSource::Deinitialize();
		}
	}
}