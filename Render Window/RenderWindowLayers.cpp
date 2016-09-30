#include "RenderWindowLayers.h"
#include "RenderWindowManager.h"
#include "ToolbarOSDLayer.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"

namespace cse
{
	namespace renderWindow
	{
		RenderWindowLayerManager::GlobalEventSink::GlobalEventSink(RenderWindowLayerManager* Parent) :
			Parent(Parent)
		{
			SME_ASSERT(Parent);
		}

		void RenderWindowLayerManager::GlobalEventSink::Handle(SME::MiscGunk::IEventData* Data)
		{
			const events::TypedEventSource* Source = dynamic_cast<const events::TypedEventSource*>(Data->Source);
			SME_ASSERT(Source);

			switch (Source->GetTypeID())
			{
			case events::TypedEventSource::kType_Plugin_ClearData:
				Parent->ClearCollections();
				break;
			case events::TypedEventSource::kType_RenderWindow_PlaceRef:
				{
					events::dialog::renderWindow::RenderWindowDialogEventData* Args = dynamic_cast<events::dialog::renderWindow::RenderWindowDialogEventData*>(Data);
					SME_ASSERT(Args);

					Parent->HandlePlaceRef(Args->PlacedRef);
				}

				break;
			}
		}

		RenderWindowLayerManager::Layer::Layer(const char* Name) :
			NamedReferenceCollection(Name)
		{
			Visible = true;
			Frozen = false;
		}

		RenderWindowLayerManager::Layer::Layer(const std::string& InSerialized) :
			NamedReferenceCollection(InSerialized)
		{
			Visible = true;
			Frozen = false;
		}

		void RenderWindowLayerManager::Layer::SetVisible(bool State)
		{
			Visible = State;
		}

		void RenderWindowLayerManager::Layer::SetFrozen(bool State)
		{
			Frozen = State;
		}

		bool RenderWindowLayerManager::Layer::IsVisible() const
		{
			return Visible;
		}

		bool RenderWindowLayerManager::Layer::IsFrozen() const
		{
			return Frozen;
		}

		void RenderWindowLayerManager::StandardOutput(const char* Fmt, ...) const
		{
			;//
		}

		const char* RenderWindowLayerManager::GetSaveFileName() const
		{
			return "RenderWindowLayers";
		}

		NamedReferenceCollection* RenderWindowLayerManager::DeserializeCollection(const std::string& In) const
		{
			return new Layer(In);
		}

		bool RenderWindowLayerManager::AddLayer(const char* Name)
		{
			if (!_stricmp(Name, DefaultLayer->GetName()))
				return false;

			NamedReferenceCollection* Existing = LookupCollection(Name);
			if (Existing)
				return false;
			else
			{
				NamedReferenceCollection* NewLayer = new Layer(Name);
				RegisterCollection(NewLayer, false);
				return true;
			}
		}

		void RenderWindowLayerManager::RemoveLayer(Layer* Layer)
		{
			SME_ASSERT(Layer != DefaultLayer);

			DeregisterCollection(Layer, true);
		}

		void RenderWindowLayerManager::SetActiveLayer(Layer* Layer)
		{
			ActiveLayer = Layer;
		}

		void RenderWindowLayerManager::AddReferenceToLayer(TESObjectREFR* Ref, Layer* To)
		{
			SME_ASSERT(To);

			if (To == DefaultLayer)
			{
				// remove it from its parent layer
				RemoveReference(Ref);
				return;
			}

			NamedReferenceCollection* Parent = GetParentCollection(Ref);
			if (Parent != To)
			{
				RemoveReference(Ref);
				AddReference(Ref, To);
			}
		}

		void RenderWindowLayerManager::AddReferenceToLayer(TESRenderSelection* Selection, Layer* To)
		{
			for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
				AddReferenceToLayer(Ref, To);
			}
		}

		void RenderWindowLayerManager::RemoveReferenceFromParentLayer(TESObjectREFR* Ref)
		{
			RemoveReference(Ref);
		}

		void RenderWindowLayerManager::RemoveReferenceFromParentLayer(TESRenderSelection* Selection)
		{
			for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
				RemoveReferenceFromParentLayer(Ref);
			}
		}

		bool RenderWindowLayerManager::RenderModalNewLayer(RenderWindowOSD*, ImGuiDX9*, void*)
		{
			static char NewNameBuffer[0x100] = { 0 };

			bool EnterKey = false;
			if (ImGui::InputText("Name", NewNameBuffer, sizeof(NewNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				EnterKey = true;

			bool InvalidName = LookupCollection(NewNameBuffer) || !_stricmp(NewNameBuffer, DefaultLayer->GetName());
			if (InvalidName)
				ImGui::Text("Layer already exists!");
			else if (strlen(NewNameBuffer) == 0)
				InvalidName = true;

			ImGui::Separator();
			bool Close = false;
			if (ImGui::IsKeyReleased(VK_ESCAPE))
				Close = true;
			else
			{
				if (ImGui::Button("OK", ImVec2(120, 0)) || EnterKey)
				{
					if (InvalidName == false)
					{
						bool Added = AddLayer(NewNameBuffer);
						SME_ASSERT(Added);
						Close = true;
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0)))
					Close = true;
			}

			if (Close)
			{
				// clear the buffer for the next use
				ZeroMemory(NewNameBuffer, sizeof(NewNameBuffer));
				return true;
			}
			else
				return false;
		}

		RenderWindowLayerManager::LayerListResult::LayerListResult()
		{
			Selection = nullptr;
			AddToLayer = RemoveLayer = SetActive = false;
		}

		void RenderWindowLayerManager::RenderPopupButton()
		{
			const ImVec4 MainColor(0, 0, 0, 0);

			ImGui::PushStyleColor(ImGuiCol_Button, MainColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, MainColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, MainColor);

			ImGui::Button(ICON_MD_LAYERS "##popupbtn_layers", ImVec2(0, 0));
			if (ImGui::IsItemHoveredRect())
				ImGui::SetTooltip("Layers");

			ImGui::PopStyleColor(3);
		}

		void RenderWindowLayerManager::RenderPopupContents()
		{
			FilterHelper.Draw();
			const TESObjectREFRArrayT& ActiveRefs = _RENDERWIN_MGR.GetActiveRefs();

			ImGui::BeginChild("contents_child_frame", ImVec2(0, 500));
			{
				ImGui::Columns(2, "ref_table_header", false);
				{
					ImGui::TextWrapped("Active Layer: %s (?)", ActiveLayer->GetName());
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Newly created references are added to this layer.\n\nRight click on a layer to show the context menu.\nTo remove references from a layer, add them to the Default layer.");

					ImGui::NextColumn();

					if (ImGui::Button(ICON_MD_ADD_TO_PHOTOS "##newlayer_btn"))
					{
						ModalWindowProviderOSDLayer::Instance.ShowModal("New Layer",
																		std::bind(&RenderWindowLayerManager::RenderModalNewLayer, this,
																				  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
																		nullptr,
																		ImGuiWindowFlags_AlwaysAutoResize);
					}

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("New Layer");

					ImGui::NextColumn();
				}
				ImGui::Columns();

				// cache active layers
				std::unordered_set<Layer*> ActiveLayers;
				for (auto Itr : ActiveRefs)
				{
					if (ReferenceTable.count(Itr->formID))
						ActiveLayers.insert(static_cast<Layer*>(ReferenceTable.at(Itr->formID)));
				}

				LayerListResult CurrentLayersResult;
				if (ImGui::CollapsingHeader("Current Layers", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Separator();
					ImGui::Columns(4, "current_layers_list", false);
					{
						ImGui::SetColumnOffset(1, 180);
						ImGui::SetColumnOffset(2, 230);
						ImGui::SetColumnOffset(3, 280);

						ImGui::Text("Name"); ImGui::NextColumn();
						ImGui::Text("Count"); ImGui::NextColumn();
						ImGui::Text(" " ICON_MD_VISIBILITY); ImGui::NextColumn();
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Visible");
						ImGui::Text(" " ICON_MD_LOCK_OUTLINE); ImGui::NextColumn();
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Frozen");
						ImGui::Separator();
					}
					ImGui::Columns();

					ImGui::BeginChild("current_layers_list_child_frame", ImVec2(0, 170));
					{
						ImGui::Columns(4, "current_layers_list", false);
						{
							ImGui::SetColumnOffset(1, 180);
							ImGui::SetColumnOffset(2, 230);
							ImGui::SetColumnOffset(3, 280);

							if (AddLayerToOSDList(DefaultLayer, CurrentLayersResult) == false)
							{
								for (auto Itr : ActiveLayers)
								{
									if (AddLayerToOSDList(Itr, CurrentLayersResult))
										break;
								}
							}
						}
						ImGui::Columns();
					}
					ImGui::EndChild();
				}

				if (HandleLayerListResult(CurrentLayersResult) == false)
				{
					LayerListResult OtherLayersResult;
					if (ImGui::CollapsingHeader("Other Layers", ImGuiTreeNodeFlags_DefaultOpen))
					{
						ImGui::Separator();
						ImGui::Columns(4, "other_layers_list", false);
						{
							ImGui::SetColumnOffset(1, 180);
							ImGui::SetColumnOffset(2, 230);
							ImGui::SetColumnOffset(3, 280);

							ImGui::Text("Name"); ImGui::NextColumn();
							ImGui::Text("Count"); ImGui::NextColumn();
							ImGui::Text(" " ICON_MD_VISIBILITY); ImGui::NextColumn();
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("Visible");
							ImGui::Text(" " ICON_MD_LOCK_OUTLINE); ImGui::NextColumn();
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("Frozen");
							ImGui::Separator();
						}
						ImGui::Columns();

						ImGui::BeginChild("other_layers_list_child_frame", ImVec2(0, 150));
						{
							ImGui::Columns(4, "other_layers_list", false);
							{
								ImGui::SetColumnOffset(1, 180);
								ImGui::SetColumnOffset(2, 230);
								ImGui::SetColumnOffset(3, 280);

								for (auto& Itr : RegisteredCollections)
								{
									Layer* ThisLayer = static_cast<Layer*>(Itr.get());
									if (ActiveLayers.count(ThisLayer) == 0)
									{
										if (AddLayerToOSDList(ThisLayer, OtherLayersResult))
											break;
									}
								}
							}
							ImGui::Columns();
						}
						ImGui::EndChild();
					}

					HandleLayerListResult(OtherLayersResult);
				}
			}
			ImGui::EndChild();
		}

		bool RenderWindowLayerManager::AddLayerToOSDList(Layer* ToAdd, LayerListResult& OutResult)
		{
			if (FilterHelper.PassFilter(ToAdd->GetName()) == false)
				return false;

			bool Handled = false;
			char Buffer[0x100] = {0};
			FORMAT_STR(Buffer, "%s_current", ToAdd->GetName());
			ImGui::PushID(Buffer);
			{
				if (ImGui::Selectable(ToAdd->GetName(), false, ImGuiSelectableFlags_AllowDoubleClick) &&
					ImGui::IsMouseDoubleClicked(0))
				{
					ToAdd->ConvertToSelection(_RENDERSEL, true, true);
				}

				if (ImGui::BeginPopupContextItem("layer_context_menu"))
				{
					if (ImGui::Selectable("Add Selection to Layer"))
					{
						OutResult.Selection = ToAdd;
						OutResult.AddToLayer = true;
						Handled = true;
					}

					if (ToAdd != DefaultLayer && ImGui::Selectable("Remove Layer"))
					{
						OutResult.Selection = ToAdd;
						OutResult.RemoveLayer = true;
						Handled = true;
					}

					if (ImGui::Selectable("Set As Active"))
					{
						OutResult.Selection = ToAdd;
						OutResult.SetActive = true;
						Handled = true;
					}

					ImGui::EndPopup();
				}

				ImGui::NextColumn();
				if (ToAdd == DefaultLayer)
					ImGui::Text("");
				else
					ImGui::Text("%d", ToAdd->GetSize());
				ImGui::NextColumn();

				bool Visible = ToAdd->IsVisible(), Frozen = ToAdd->IsFrozen();
				if (ImGui::Checkbox("##visible_toggle", &Visible))
					ToAdd->SetVisible(Visible);
				ImGui::NextColumn();
				if (ImGui::Checkbox("##frozen_toggle", &Frozen))
					ToAdd->SetFrozen(Frozen);
				ImGui::NextColumn();
			}
			ImGui::PopID();


			return Handled;
		}

		bool RenderWindowLayerManager::HandleLayerListResult(LayerListResult& Result)
		{
			if (Result.Selection)
			{
				if (Result.AddToLayer)
					AddReferenceToLayer(_RENDERSEL, Result.Selection);
				else if (Result.RemoveLayer)
					RemoveLayer(Result.Selection);
				else if (Result.SetActive)
					SetActiveLayer(Result.Selection);

				return true;
			}

			return false;
		}

		void RenderWindowLayerManager::HandlePlaceRef(TESObjectREFR* Ref)
		{
			SME_ASSERT(ActiveLayer);

			if (ActiveLayer != DefaultLayer)
				AddReferenceToLayer(Ref, ActiveLayer);
		}

		RenderWindowLayerManager::RenderWindowLayerManager() :
			NamedReferenceCollectionManager(kValidationPolicy_Default),
			ActiveLayer(nullptr),
			FilterHelper()
		{
			EventSink = new GlobalEventSink(this);
			DefaultLayer = new Layer("Default");
			ActiveLayer = DefaultLayer;
		}

		RenderWindowLayerManager::~RenderWindowLayerManager()
		{
			SAFEDELETE(EventSink);
			SAFEDELETE(DefaultLayer);
		}

		void RenderWindowLayerManager::Initialize()
		{
			NamedReferenceCollectionManager::Initialize();

			events::dialog::renderWindow::kPlaceRef.AddSink(EventSink);

			ToolbarOSDLayer::Instance.RegisterTopToolbarButton("##popup_reference_layers",
															   std::bind(&RenderWindowLayerManager::RenderPopupButton, this),
															   std::bind(&RenderWindowLayerManager::RenderPopupContents, this));
		}

		void RenderWindowLayerManager::Deinitialize()
		{
			events::dialog::renderWindow::kPlaceRef.RemoveSink(EventSink);

			NamedReferenceCollectionManager::Deinitialize();
		}

		bool RenderWindowLayerManager::IsParentLayerVisible(TESObjectREFR* Ref) const
		{
			Layer* Parent = static_cast<Layer*>(GetParentCollection(Ref));
			if (Parent)
				return Parent->IsVisible();
			else
				return DefaultLayer->IsVisible();
		}

		bool RenderWindowLayerManager::IsParentLayerFrozen(TESObjectREFR* Ref) const
		{
			Layer* Parent = static_cast<Layer*>(GetParentCollection(Ref));
			if (Parent)
				return Parent->IsFrozen();
			else
				return DefaultLayer->IsFrozen();
		}

		const char* RenderWindowLayerManager::GetParentLayerName(TESObjectREFR* Ref) const
		{
			Layer* Parent = static_cast<Layer*>(GetParentCollection(Ref));
			if (Parent)
				return Parent->GetName();
			else
				return nullptr;
		}
	}
}