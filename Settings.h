#pragma once

namespace cse
{
	namespace settings
	{
		void									Register(bgsee::INISettingDepotT& Depot);

		namespace dialogs
		{
			extern INISetting					kRenderWindowState;
			extern INISetting					kObjectWindowState;
			extern INISetting					kCellViewWindowState;

			extern INISetting					kSortFormListsByActiveForm;

			extern INISetting					kColorizeActiveForms;
			extern INISetting					kActiveFormForeColor;
			extern INISetting					kActiveFormBackColor;

			extern INISetting					kShowMainWindowsInTaskbar;
			extern INISetting					kShowEditDialogsInTaskbar;

			extern INISetting					kColorizeFormOverrides;
			extern INISetting					kFormOverrideLevel0ForeColor;
			extern INISetting					kFormOverrideLevel0BackColor;
			extern INISetting					kFormOverrideLevel1ForeColor;
			extern INISetting					kFormOverrideLevel1BackColor;
			extern INISetting					kFormOverrideLevel2ForeColor;
			extern INISetting					kFormOverrideLevel2BackColor;
			extern INISetting					kFormOverrideLevel3ForeColor;
			extern INISetting					kFormOverrideLevel3BackColor;

			extern INISetting					kMultiplePreviewWindows;
			extern INISetting					kPreserveEditorDialogLocations;
		}

		namespace general
		{
			extern INISetting					kShowNumericEditorIDWarning;
			extern INISetting					kCrashHandlerMode;
			extern INISetting					kFaceGenPreviewResponseWindow;
			extern INISetting					kFaceGenPreviewVoiceDelay;
			extern INISetting					kDisableSplashScreen;
			extern INISetting					kShowHallOfFameMembersInTitleBar;
		}

		namespace lod
		{
			extern INISetting					kPartialTextureResolution;
			extern INISetting					kDeletePartialsAfterGeneration;
		}

		namespace plugins
		{
			extern INISetting					kPreventTimeStampChanges;
			extern INISetting					kSaveLoadedESPsAsMasters;
		}

		namespace renderer
		{
			extern INISetting					kAltRefMovementSpeed;
			extern INISetting					kAltRefSnapGrid;
			extern INISetting					kAltRefRotationSpeed;
			extern INISetting					kAltRefSnapAngle;
			extern INISetting					kAltCamRotationSpeed;
			extern INISetting					kAltCamZoomSpeed;
			extern INISetting					kAltCamPanSpeed;
			extern INISetting					kCoplanarRefDrops;
			extern INISetting					kSwitchCAndY;
			extern INISetting					kFixedCameraPivot;
			extern INISetting					kCameraFOV;
			extern INISetting					kExteriorSnapshotResolution;
			extern INISetting					kZOffsetDuplicatedRefs;
			extern INISetting					kParentChildVisualIndicator;
			extern INISetting					kRefToggleOpacityAlpha;
			extern INISetting					kPathGridLinkedRefIndicator;
			extern INISetting					kPathGridLinkedRefIndicatorFlags;

			enum
			{
				kPathGridLinkedRefIndicatorFlag_HidePointBoundingBox	=	1 << 0,
				kPathGridLinkedRefIndicatorFlag_HideLinkedRefNode		=	1 << 1,
				kPathGridLinkedRefIndicatorFlag_HideLineConnector		=	1 << 2,
			};
		}

		namespace renderWindowOSD
		{
			extern INISetting					kShowSelectionStats;
			extern INISetting					kShowRAMUsage;
			extern INISetting					kShowMouseRef;
			extern INISetting					kMouseRefCtrlModified;

			extern INISetting					kColorSelectionStats;
			extern INISetting					kColorRAMUsage;
			extern INISetting					kColorMouseRef;
			extern INISetting					kColorNotifications;
		}

		namespace renderWindowFlyCamera
		{
			extern INISetting					kMovementSpeed;
			extern INISetting					kSprintMultiplier;
			extern INISetting					kCrawlMultiplier;
			extern INISetting					kRotationSpeed;
			extern INISetting					kCameraFOV;
		}

		namespace startup
		{
			extern INISetting					kLoadPlugin;
			extern INISetting					kPluginName;
			extern INISetting					kOpenScriptWindow;
			extern INISetting					kScriptEditorID;
			extern INISetting					kSetWorkspace;
			extern INISetting					kWorkspacePath;
		}

		namespace versionControl
		{
			extern INISetting					kBackupOnSave;
		}
	}
}