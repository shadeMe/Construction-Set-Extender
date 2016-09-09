#pragma once
#include "RenderWindowCommon.h"

namespace cse
{
	namespace renderWindow
	{
		namespace actions
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

			class BuiltInKeyComboRWA : public IRenderWindowAction
			{
				typedef std::function<void(input::BuiltIn::KeyBinding&)>	ActionDelegateT;

				input::BuiltIn::KeyBinding		Key;
				ActionDelegateT					Delegate;
			public:
				BuiltInKeyComboRWA(std::string Name, std::string Desc, UInt8 Context, input::BuiltIn::KeyBinding Binding, ActionDelegateT Delegate = nullptr);
				virtual ~BuiltInKeyComboRWA() override;

				virtual bool		operator()() override;

				const input::BuiltIn::KeyBinding&		GetBuiltInBinding() const;
			};

			class ToggleVisibilityRWA : public IRenderWindowAction
			{
			public:
				enum : int
				{
					kType__NONE = -1,

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
					kAlign_Position = 1,
					kAlign_Rotation = 2,
				};

				enum
				{
					kAxis_X = 1 << 0,
					kAxis_Y = 1 << 1,
					kAxis_Z = 1 << 2,

					kAxis_ALL = kAxis_X | kAxis_Y | kAxis_Z,
				};
			protected:
				UInt8				Alignment;
				UInt8				Axis;
			public:
				AlignReferencesRWA(UInt8 Alignment, UInt8 Axis);
				virtual ~AlignReferencesRWA() override;

				virtual bool			operator()() override;
			};

			extern BasicRWA InvertSelection;
			extern BasicRWA SelectAll;

			extern AlignReferencesRWA AlignPosition;
			extern AlignReferencesRWA AlignRotation;

			extern BasicRWA GroupSelection;
			extern BasicRWA UngroupSelection;
			extern BasicRWA OrphanizeSelection;

			extern BasicRWA FreezeSelection;
			extern BasicRWA ThawSelection;
			extern BasicRWA ThawAll;
			extern BasicRWA ToggleFreezeInactive;

			extern BasicRWA ToggleSelectionVisibility;
			extern BasicRWA ToggleSelectionChildrenVisibility;
			extern BasicRWA RevealAll;

			extern BasicRWA DimSelectionOpacity;
			extern BasicRWA ResetSelectionOpacity;

			extern BasicRWA UnlinkPathGridSelection;

			extern BasicRWA ShowBatchEditor;
			extern BasicRWA ShowUseInfo;
			extern BasicRWA ShowSearchReplace;

			extern BasicRWA ToggleAuxViewport;

			extern BasicRWA ToggleStaticCameraPivot;
			extern BasicRWA ToggleAlternateMovementSettings;
			extern BasicRWA TogglePathGridEditMode;
			extern BasicRWA ToggleSnapToGrid;
			extern BasicRWA ToggleSnapToAngle;

			extern ToggleVisibilityRWA ToggleVisibility[];

			extern BasicRWA FocusOnRefFilter;
			extern BasicRWA JumpToExteriorCell;

			namespace builtIn
			{
				extern BuiltInKeyComboRWA ReloadAllPathGrids;
				extern BuiltInKeyComboRWA GenerateActiveExteriorLocalMap;
				extern BuiltInKeyComboRWA ToggleBrightLight;
				extern BuiltInKeyComboRWA ToggleLandscapeEditMode;
				extern BuiltInKeyComboRWA ToggleCollisionGeom;
				extern BuiltInKeyComboRWA RefreshRenderWindow;
				extern BuiltInKeyComboRWA MoveCamYAxisPos;
				extern BuiltInKeyComboRWA MoveCamYAxisNeg;
				extern BuiltInKeyComboRWA MoveCamXAxisPos;
				extern BuiltInKeyComboRWA MoveCamXAxisNeg;
				extern BuiltInKeyComboRWA PitchCamPos;
				extern BuiltInKeyComboRWA PitchCamNeg;
				extern BuiltInKeyComboRWA YawCamPos;
				extern BuiltInKeyComboRWA YawCamNeg;
				extern BuiltInKeyComboRWA RollCamPos;
				extern BuiltInKeyComboRWA RollCamNeg;
				extern BuiltInKeyComboRWA ShowLandUseDialog;
				extern BuiltInKeyComboRWA ShowSelectedRefInfo;
				extern BuiltInKeyComboRWA LinkPathGridSelection;
				extern BuiltInKeyComboRWA ToggleMarkers;
				extern BuiltInKeyComboRWA ToggleWater;
				extern BuiltInKeyComboRWA ToggleGlobalWireframe;
				extern BuiltInKeyComboRWA ToggleLandscape;
				extern BuiltInKeyComboRWA ToggleLightRadius;
				extern BuiltInKeyComboRWA Fall;
				extern BuiltInKeyComboRWA SavePlugin;
				extern BuiltInKeyComboRWA Cut;
				extern BuiltInKeyComboRWA Redo;
				extern BuiltInKeyComboRWA Undo;
				extern BuiltInKeyComboRWA UndoEx;
				extern BuiltInKeyComboRWA CullLightSelection;
				extern BuiltInKeyComboRWA ToggleObjects;
				extern BuiltInKeyComboRWA AutoLinkPathGridSelection;
				extern BuiltInKeyComboRWA Copy;
				extern BuiltInKeyComboRWA FallCam;
				extern BuiltInKeyComboRWA CenterCam;
				extern BuiltInKeyComboRWA ToggleVertexColoring;
				extern BuiltInKeyComboRWA TopCamera;
				extern BuiltInKeyComboRWA Duplicate;
				extern BuiltInKeyComboRWA DeselectAll;
				extern BuiltInKeyComboRWA RunHavokSim;
				extern BuiltInKeyComboRWA Delete;
				extern BuiltInKeyComboRWA ToggleSelectionWireframe;
				extern BuiltInKeyComboRWA CheckBounds;
				extern BuiltInKeyComboRWA ToggleCellBorders;
				extern BuiltInKeyComboRWA Paste;
				extern BuiltInKeyComboRWA PasteInPlace;
			}
		}
	}
}