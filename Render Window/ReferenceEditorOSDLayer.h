#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class ReferenceEditorOSDLayer;

		class IReferenceEditComponent
		{
		public:
			struct DrawContext
			{
				const OSDLayerStateData&	ParentState;
				RenderWindowOSD*			OSD;
				ImGuiDX9*					GUI;
				const TESObjectREFRArrayT&	Selection;

				DrawContext(const OSDLayerStateData& ParentState, RenderWindowOSD* OSD, ImGuiDX9* GUI, const TESObjectREFRArrayT& Selection);

				bool SelectionHasInvalidType(UInt8 ValidFormType) const;
				bool HasMultipleSelection() const;
			};

			virtual ~IReferenceEditComponent() = default;

			virtual bool SupportsReference(TESObjectREFR* Ref) const = 0;
			virtual void Draw(DrawContext* Context) = 0;
		};

		class Reference3DEditComponent : public IReferenceEditComponent
		{
			enum
			{
				kTransformationMode_Global = 0,
				kTransformationMode_Local = 1,
			};

			int		TransformationMode;
			bool	AlignmentAxisX;
			bool	AlignmentAxisY;
			bool	AlignmentAxisZ;
		protected:
			bool Vector3Equals(const Vector3& a, const Vector3& b) const;
			void BuildForSingleSelection(DrawContext* Context);
			void BuildForMultipleSelection(DrawContext* Context);

			void MoveSelection(bool X, bool Y, bool Z) const;
			void RotateSelection(bool Local, bool X, bool Y, bool Z) const;
			void ScaleSelection(bool Local) const;
			void AlignSelection(bool Position, bool Rotation) const;
			void SaveSelectionToUndoStack() const;
			void DrawDragTrail(DrawContext* Context) const;
		public:
			Reference3DEditComponent();
			virtual ~Reference3DEditComponent() override = default;

			virtual bool SupportsReference(TESObjectREFR* Ref) const override;
			virtual void Draw(DrawContext* Context) override;

		};

		class ReferenceFlagsEditComponent : public IReferenceEditComponent
		{
		public:
			virtual ~ReferenceFlagsEditComponent() override = default;

			virtual bool SupportsReference(TESObjectREFR* Ref) const override;
			virtual void Draw(DrawContext* Context) override;
		};

		class ReferenceEnableParentEditComponent : public IReferenceEditComponent
		{
		public:
			virtual ~ReferenceEnableParentEditComponent() override = default;

			virtual bool SupportsReference(TESObjectREFR* Ref) const override;
			virtual void Draw(DrawContext* Context) override;
		};

		class ReferenceOwnershipEditComponent : public IReferenceEditComponent
		{
			enum
			{
				kOwnership_None = 0,
				kOwnership_NPC,
				kOwnership_Faction,
				kOwnership_Multiple,
				kOwnership__MAX,
			};

			enum
			{
				kSecondParam_Global = 0,
				kSecondParam_Rank,
				kSecondParam__MAX
			};

			enum class SelectionType
			{
				NPC,
				Faction,
				Global,
				Rank,
			};

			enum class SelectionPopupState
			{
				NotShowing,
				Showing,
				OK,
				Cancel,
			};

			struct
			{
				std::string			PopupName;
				ImGuiTextFilter		Filter;
				SelectionPopupState	PopupState = SelectionPopupState::NotShowing;
				SelectionType		Type = SelectionType::NPC;
				TESForm*			FormParam = nullptr;
				struct
				{
					TESForm* Form = nullptr;
					SInt32	Rank = -1;
				} SelectedItem;
			} SelectionState;

			struct
			{
				std::unordered_set<TESForm*>
						LastRefSelection;
				bool	RefSelectionChanged = false;
				int		LastOwnershipSelection = kOwnership_None;
				struct
				{
					TESForm*	Owner = nullptr;
					TESGlobal*	Global = nullptr;
					SInt32		Rank = -1;
				} BaselineOwnershipData;
			} CachedData;

			void UpdateCachedData(DrawContext* Context);
			void BeginSelectionPopup(SelectionType Type, TESForm* FormParam = nullptr);
			bool DrawSelectionPopup();
			void EndSelectionPopup();
		public:
			virtual ~ReferenceOwnershipEditComponent() override = default;

			virtual bool SupportsReference(TESObjectREFR* Ref) const override;
			virtual void Draw(DrawContext* Context) override;
		};

		class ReferenceExtraDataEditComponent : public IReferenceEditComponent
		{
			bool	HasExtraDataCharge(TESObjectREFR* Ref) const;
			bool	HasExtraDataHealth(TESObjectREFR* Ref) const;
			bool	HasExtraDataTimeLeft(TESObjectREFR* Ref) const;
			bool	HasExtraDataCount(TESObjectREFR* Ref) const;
			bool	HasExtraDataSoul(TESObjectREFR* Ref) const;
		public:
			virtual ~ReferenceExtraDataEditComponent() override = default;

			virtual bool SupportsReference(TESObjectREFR* Ref) const override;
			virtual void Draw(DrawContext* Context) override;
		};

		class ReferenceEditorOSDLayer : public IRenderWindowOSDLayer
		{
			using RefSelectionArrayT = std::vector<std::pair<TESObjectREFR*, bool>>;
			using EditComponentArrayT = std::vector<std::unique_ptr<IReferenceEditComponent>>;

			OSDLayerStateData		WindowState;
			ImGuiTextFilter			RefListFilterHelper;
			EditComponentArrayT		EditComponents;
			RefSelectionArrayT		CurrentSelection;
			bool					RefListOnlyShowSelection;
			TESObjectREFR*			RefListContextMenuSelection;

			void UpdateCurrentSelection();
			void BuildReferenceListChildWindow(RefSelectionArrayT& References);
			bool BuildEditComponentTabs(RefSelectionArrayT& References, RenderWindowOSD* OSD, ImGuiDX9* GUI);
		public:
			ReferenceEditorOSDLayer();
			virtual ~ReferenceEditorOSDLayer();

			virtual void Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool NeedsBackgroundUpdate();

			static ReferenceEditorOSDLayer			Instance;
		};
	}
}