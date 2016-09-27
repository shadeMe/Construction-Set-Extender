#pragma once
#include "RenderWindowCommon.h"
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowLayerManager : public NamedReferenceCollectionManager
		{
			class GlobalEventSink : public SME::MiscGunk::IEventSink
			{
				RenderWindowLayerManager*		Parent;
			public:
				GlobalEventSink(RenderWindowLayerManager* Parent);

				virtual void					Handle(SME::MiscGunk::IEventData* Data);
			};

			friend class GlobalEventSink;

			class Layer : public NamedReferenceCollection
			{
			protected:
				bool				Visible;
				bool				Frozen;
			public:
				Layer(const char* Name);
				explicit Layer(const std::string& InSerialized);
				inline virtual ~Layer() = default;

				void				SetVisible(bool State);
				void				SetFrozen(bool State);

				bool				IsVisible() const;
				bool				IsFrozen() const;
			};

			virtual void							StandardOutput(const char* Fmt, ...) const override;
			virtual const char*						GetSaveFileName() const override;
			virtual NamedReferenceCollection*		DeserializeCollection(const std::string& In) const override;

			bool					AddLayer(const char* Name);			// returns false if there's a name collision
			void					RemoveLayer(Layer* Layer);
			void					SetActiveLayer(Layer* Layer);

			void					AddReferenceToLayer(TESObjectREFR* Ref, Layer* To);	// automatically moves the ref from its current layer
			void					AddReferenceToLayer(TESRenderSelection* Selection, Layer* To);
			void					RemoveReferenceFromParentLayer(TESObjectREFR* Ref);
			void					RemoveReferenceFromParentLayer(TESRenderSelection* Selection);

			struct LayerListResult
			{
				Layer*		Selection;
				bool		AddToLayer;
				bool		RemoveLayer;
				bool		SetActive;

				LayerListResult();
			};

			bool					RenderModalNewLayer(RenderWindowOSD*, ImGuiDX9*, void*);
			void					RenderPopupButton();
			void					RenderPopupContents();
			bool					AddLayerToOSDList(Layer* ToAdd, LayerListResult& OutResult);		// returns true if handled
			bool					HandleLayerListResult(LayerListResult& Result);						// returns true if handled

			void					HandlePlaceRef(TESObjectREFR* Ref);

			GlobalEventSink*		EventSink;
			Layer*					ActiveLayer;
			Layer*					DefaultLayer;
			ImGuiTextFilter			FilterHelper;
		public:
			RenderWindowLayerManager();
			virtual ~RenderWindowLayerManager() override;

			virtual void		Initialize() override;
			virtual void		Deinitialize() override;

			bool				IsParentLayerVisible(TESObjectREFR* Ref) const;		// if the ref has no parent layer, returns the state of the Default layer
			bool				IsParentLayerFrozen(TESObjectREFR* Ref) const;		// ditto
		};
	}
}