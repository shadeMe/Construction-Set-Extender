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
				std::string				Name;
				std::string				Description;
			public:
				IRenderWindowAction(std::string Name, std::string Desc);
				virtual ~IRenderWindowAction() = 0
				{
					;//
				}

				virtual void			operator()() = 0;
				virtual const char*		GetName() const;
				virtual const char*		GetDescription() const;
			};

			namespace impl
			{
				class BasicRWA : public IRenderWindowAction
				{
					typedef std::function<void()>		ActionDelegateT;

					ActionDelegateT		Delegate;
				public:
					BasicRWA(std::string Name, std::string Desc, ActionDelegateT Delegate);
					virtual ~BasicRWA();

					virtual void		operator()() override;
				};

				class ToggleINISettingRWA : public IRenderWindowAction
				{
					SME::INI::INISetting&	Setting;
				public:
					ToggleINISettingRWA(std::string Name, std::string Desc, SME::INI::INISetting& Setting);
					virtual ~ToggleINISettingRWA();

					virtual void		operator()() override;
				};

				class ToggleVisibilityRWA : public IRenderWindowAction
				{
				public:
					enum : int
					{
						kType__NONE		= -1,

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

			extern impl::ToggleVisibilityRWA ToggleVisibility[];

			extern impl::BasicRWA FocusOnRefFilter;
			extern impl::BasicRWA JumpToExteriorCell;
		}
	}
}