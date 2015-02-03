#pragma once

namespace ConstructionSetExtender
{
	namespace Settings
	{
		void									Register(BGSEditorExtender::INISettingDepotT& Depot);

		namespace Dialogs
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
		}

		namespace General
		{
			extern INISetting					kShowNumericEditorIDWarning;
			extern INISetting					kCrashHandlerMode;
			extern INISetting					kFaceGenPreviewResponseWindow;
			extern INISetting					kFaceGenPreviewVoiceDelay;
			extern INISetting					kDisableSplashScreen;
			extern INISetting					kShowHallOfFameMembersInTitleBar;
		}

		namespace LOD
		{
			extern INISetting					kPartialTextureResolution;
			extern INISetting					kDeletePartialsAfterGeneration;
		}

		namespace Plugins
		{
			extern INISetting					kPreventTimeStampChanges;
			extern INISetting					kSaveLoadedESPsAsMasters;
		}

		namespace Renderer
		{
			extern INISetting					kUpdatePeriod;
			extern INISetting					kUpdateViewPortAsync;
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
		}

		namespace RenderWindowPainter
		{
			extern INISetting					kShowSelectionStats;
			extern INISetting					kShowRAMUsage;
			extern INISetting					kShowMouseRef;
			extern INISetting					kMouseRefCtrlModified;
			extern INISetting					kFontFace;
			extern INISetting					kFontSize;
		}

		namespace RenderWindowFlyCamera
		{
			extern INISetting					kMovementSpeed;
			extern INISetting					kSprintMultiplier;
			extern INISetting					kCrawlMultiplier;
			extern INISetting					kRotationSpeed;
			extern INISetting					kCameraFOV;
		}

		namespace Startup
		{
			extern INISetting					kLoadPlugin;
			extern INISetting					kPluginName;
			extern INISetting					kOpenScriptWindow;
			extern INISetting					kScriptEditorID;
			extern INISetting					kSetWorkspace;
			extern INISetting					kWorkspacePath;
		}

		namespace VersionControl
		{
			extern INISetting					kBackupOnSave;
		}
	}
}