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
			public:
				// render window modes in which the action can be executed
				enum
				{
					kMode_ReferenceEdit		= 1 << 0,
					kMode_PathGridEdit		= 1 << 1,
					kMode_LandscapeEdit		= 1 << 2,

					kMode_All				= kMode_ReferenceEdit | kMode_PathGridEdit | kMode_LandscapeEdit,
				};
			protected:
				std::string				Name;
				std::string				Description;
				UInt8					ExecutionContext;
			public:
				IRenderWindowAction(std::string Name, std::string Desc, UInt8 Context);
				virtual ~IRenderWindowAction() = 0
				{
					;//
				}

				virtual bool			operator()() = 0;			// returns false if the operation was not performed, true otherwise
				virtual const char*		GetName() const;
				virtual const char*		GetDescription() const;

				bool					IsExecutableInReferenceEdit() const;
				bool					IsExecutableInPathGridEdit() const;
				bool					IsExecutableInLandscapeEdit() const;
				bool					IsExecutableInCurrentContext() const;
				bool					HasSameExecutionContext(const IRenderWindowAction& RHS);	// returns true if any of the RHS' contexts match
			};

			namespace impl
			{
				class BasicRWA : public IRenderWindowAction
				{
					typedef std::function<void()>		ActionDelegateT;

					ActionDelegateT		Delegate;
				public:
					BasicRWA(std::string Name, std::string Desc, ActionDelegateT Delegate);
					BasicRWA(std::string Name, std::string Desc, UInt8 Context, ActionDelegateT Delegate);
					virtual ~BasicRWA() override;

					virtual bool		operator()() override;
				};

				class ToggleINISettingRWA : public IRenderWindowAction
				{
					SME::INI::INISetting&	Setting;
				public:
					ToggleINISettingRWA(std::string Name, std::string Desc, UInt8 Context, SME::INI::INISetting& Setting);
					virtual ~ToggleINISettingRWA() override;

					virtual bool		operator()() override;
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
					virtual ~ToggleVisibilityRWA() override;

					virtual bool			operator()() override;

					static bool				IsVisible(int Type);
				};

				class AlignReferencesRWA : public IRenderWindowAction
				{
				public:
					enum
					{
						kAlign_Position		= 1,
						kAlign_Rotation		= 2,
					};

					enum
					{
						kAxis_X		= 1 << 0,
						kAxis_Y		= 1 << 1,
						kAxis_Z		= 1 << 2,

						kAxis_ALL	= kAxis_X | kAxis_Y | kAxis_Z,
					};
				protected:
					UInt8				Alignment;
					UInt8				Axis;
				public:
					AlignReferencesRWA(UInt8 Alignment, UInt8 Axis);
					virtual ~AlignReferencesRWA() override;

					virtual bool			operator()() override;
				};
			}

			extern impl::BasicRWA InvertSelection;
			extern impl::BasicRWA SelectAll;

			extern impl::AlignReferencesRWA AlignPosition;
			extern impl::AlignReferencesRWA AlignRotation;

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

			extern impl::BasicRWA LinkPathGridSelection;
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