#pragma once

namespace cse
{
	namespace renderWindow
	{
		namespace actions
		{
			// functor that performs a render window action/operation
			class IRenderWindowAction
			{
			protected:
				std::string				Description;
			public:
				IRenderWindowAction(std::string Desc);
				virtual ~IRenderWindowAction() = 0
				{
					;//
				}

				virtual void			operator()() = 0;
				virtual const char*		GetDescription() const;
			};

			namespace impl
			{
				class BasicRWA : public IRenderWindowAction
				{
					typedef std::function<void()>		ActionDelegateT;

					ActionDelegateT		Delegate;
				public:
					BasicRWA(std::string Desc, ActionDelegateT Delegate);
					virtual ~BasicRWA();

					virtual void		operator()() override;
				};

				class ToggleINISettingRWA : public IRenderWindowAction
				{
					SME::INI::INISetting&	Setting;
				public:
					ToggleINISettingRWA(std::string Desc, SME::INI::INISetting& Setting);
					virtual ~ToggleINISettingRWA();

					virtual void		operator()() override;
				};

				class ToggleVisibilityRWA : public IRenderWindowAction
				{
				public:
					enum : int
					{
						kType__NONE		= 0,

						kType_Objects,
						kType_Markers,
						kType_Wireframe,
						kType_BrightLight,
						kType_Sky,
						kType_SolidSubspaces,
						kType_CollisionGeom,
						kType_Leaves,
						kType_Trees,
						kType_Water,
						kType_CellBorders,
						kType_Land,
						kType_LightRadius,
						kType_ParentChildIndicator,
						kType_PathGridLinkedRefIndicator,
						kType_InitiallyDisabledRefs,
						kType_InitiallyDisabledRefsChildren,
						kType_GrassOverlay,

						kType__MAX
					};
				private:
					int				Type;
				public:
					ToggleVisibilityRWA(int Type);
					virtual ~ToggleVisibilityRWA();

					virtual void			operator()() override;

					static bool				IsVisible(int Type);
				};
			}

			extern impl::BasicRWA InvertSelection;
			extern impl::BasicRWA SelectAll;

			extern impl::BasicRWA GroupSelection;
			extern impl::BasicRWA UngroupSelection;
			extern impl::BasicRWA OrphanizeSelection;

			extern impl::BasicRWA FreezeSelection;
			extern impl::BasicRWA ThawSelection;
			extern impl::BasicRWA ThawAll;
			extern impl::BasicRWA ToggleFreezeInactive;

			extern impl::BasicRWA ToggleSelectionVisibility;
			extern impl::BasicRWA ToggleSelectionChildrenVisibility;
			extern impl::BasicRWA RevealAll;

			extern impl::BasicRWA DimSelectionOpacity;
			extern impl::BasicRWA ResetSelectionOpacity;

			extern impl::BasicRWA UnlinkPathGridSelection;

			extern impl::BasicRWA ShowBatchEditor;
			extern impl::BasicRWA ShowUseInfo;
			extern impl::BasicRWA ShowSearchReplace;

			extern impl::BasicRWA ToggleAuxViewport;

			extern impl::ToggleINISettingRWA ToggleStaticCameraPivot;
			extern impl::BasicRWA ToggleAlternateMovementSettings;
			extern impl::BasicRWA TogglePathGridEditMode;
			extern impl::BasicRWA ToggleSnapToGrid;
			extern impl::BasicRWA ToggleSnapToAngle;

			extern impl::ToggleVisibilityRWA ToggleVisibilityObjects;
			extern impl::ToggleVisibilityRWA ToggleVisibilityMarkers;
			extern impl::ToggleVisibilityRWA ToggleVisibilityWireframe;
			extern impl::ToggleVisibilityRWA ToggleVisibilityBrightLight;
			extern impl::ToggleVisibilityRWA ToggleVisibilitySky;
			extern impl::ToggleVisibilityRWA ToggleVisibilitySolidSubspaces;
			extern impl::ToggleVisibilityRWA ToggleVisibilityCollisionGeom;
			extern impl::ToggleVisibilityRWA ToggleVisibilityLeaves;
			extern impl::ToggleVisibilityRWA ToggleVisibilityTrees;
			extern impl::ToggleVisibilityRWA ToggleVisibilityWater;
			extern impl::ToggleVisibilityRWA ToggleVisibilityLandscape;
			extern impl::ToggleVisibilityRWA ToggleVisibilityCellBorders;
			extern impl::ToggleVisibilityRWA ToggleVisibilityParentChildIndicator;
			extern impl::ToggleVisibilityRWA ToggleVisibilityPathGridLinkedRefIndicator;
			extern impl::ToggleVisibilityRWA ToggleVisibilityInitiallyDisabledRefs;
			extern impl::ToggleVisibilityRWA ToggleVisibilityInitiallyDisabledRefsChildren;
			extern impl::ToggleVisibilityRWA ToggleVisibilityGrassOverlay;

			extern impl::BasicRWA FocusOnRefFilter;
		}
	}
}