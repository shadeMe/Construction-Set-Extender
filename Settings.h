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
			extern INISetting					kSalvageActivePluginOnCrash;
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
			extern INISetting					kFixedCameraPivot;
			extern INISetting					kFixedCameraPivotMul;
			extern INISetting					kCameraFOV;
			extern INISetting					kExteriorSnapshotResolution;
			extern INISetting					kZOffsetDuplicatedRefs;
			extern INISetting					kParentChildVisualIndicator;
			extern INISetting					kRefToggleOpacityAlpha;
			extern INISetting					kPathGridLinkedRefIndicator;
			extern INISetting					kPathGridLinkedRefIndicatorFlags;
			extern INISetting					kGrassOverlayTexturePath;
			extern INISetting					kNotifyOnInvalidExecutionContext;
			extern INISetting					kUnrestrictedMouseMovement;
			extern INISetting					kShowSelectionMask;
			extern INISetting					kSelectionMaskColor;
			extern INISetting					kShowMouseOverMask;
			extern INISetting					kMouseOverMaskColor;

			enum
			{
				kPathGridLinkedRefIndicatorFlag_HidePointBoundingBox	=	1 << 0,
				kPathGridLinkedRefIndicatorFlag_HideLinkedRefNode		=	1 << 1,
				kPathGridLinkedRefIndicatorFlag_HideLineConnector		=	1 << 2,
			};
		}

		namespace renderWindowOSD
		{
			extern INISetting					kShowMouseRef;
			extern INISetting					kMouseRefCtrlModified;
			extern INISetting					kShowInfoOverlay;
			extern INISetting					kShowRefBatchEditor;
			extern INISetting					kShowToolbar;
			extern INISetting					kShowNotifications;

			extern INISetting					kFontSize;
			extern INISetting					kFontFace;

			extern INISetting					kColorText;
			extern INISetting					kColorTextDisabled;
			extern INISetting					kColorWindowBg;              // Background of normal windows
			extern INISetting					kColorChildBg;               // Background of child windows
			extern INISetting					kColorPopupBg;               // Background of popups; menus; tooltips windows
			extern INISetting					kColorBorder;
			extern INISetting					kColorBorderShadow;
			extern INISetting					kColorFrameBg;               // Background of checkbox; radio button; plot; slider; text input
			extern INISetting					kColorFrameBgHovered;
			extern INISetting					kColorFrameBgActive;
			extern INISetting					kColorTitleBg;
			extern INISetting					kColorTitleBgActive;
			extern INISetting					kColorTitleBgCollapsed;
			extern INISetting					kColorMenuBarBg;
			extern INISetting					kColorScrollbarBg;
			extern INISetting					kColorScrollbarGrab;
			extern INISetting					kColorScrollbarGrabHovered;
			extern INISetting					kColorScrollbarGrabActive;
			extern INISetting					kColorCheckMark;
			extern INISetting					kColorSliderGrab;
			extern INISetting					kColorSliderGrabActive;
			extern INISetting					kColorButton;
			extern INISetting					kColorButtonHovered;
			extern INISetting					kColorButtonActive;
			extern INISetting					kColorHeader;                // Header* colors are used for CollapsingHeader; TreeNode; Selectable; MenuItem
			extern INISetting					kColorHeaderHovered;
			extern INISetting					kColorHeaderActive;
			extern INISetting					kColorSeparator;
			extern INISetting					kColorSeparatorHovered;
			extern INISetting					kColorSeparatorActive;
			extern INISetting					kColorResizeGrip;
			extern INISetting					kColorResizeGripHovered;
			extern INISetting					kColorResizeGripActive;
			extern INISetting					kColorTab;
			extern INISetting					kColorTabHovered;
			extern INISetting					kColorTabActive;
			extern INISetting					kColorTabUnfocused;
			extern INISetting					kColorTabUnfocusedActive;
			extern INISetting					kColorPlotLines;
			extern INISetting					kColorPlotLinesHovered;
			extern INISetting					kColorPlotHistogram;
			extern INISetting					kColorPlotHistogramHovered;
			extern INISetting					kColorTableHeaderBg;         // Table header background
			extern INISetting					kColorTableBorderStrong;     // Table outer and header borders (prefer using Alpha=1.0 here)
			extern INISetting					kColorTableBorderLight;      // Table inner borders (prefer using Alpha=1.0 here)
			extern INISetting					kColorTableRowBg;            // Table row background (even rows)
			extern INISetting					kColorTableRowBgAlt;         // Table row background (odd rows)
			extern INISetting					kColorTextSelectedBg;
			extern INISetting					kColorDragDropTarget;
			extern INISetting					kColorNavHighlight;          // Gamepad/keyboard: current highlighted item
			extern INISetting					kColorNavWindowingHighlight; // Highlight window when using CTRL+TAB
			extern INISetting					kColorNavWindowingDimBg;     // Darken/colorize entire screen behind the CTRL+TAB window list; when active
			extern INISetting					kColorModalWindowDimBg;
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
			extern INISetting					kLogInstantiation;
			extern INISetting					kLogChangeSetActive;
			extern INISetting					kLogChangeSetDeleted;
			extern INISetting					kLogChangeSetFormID;
			extern INISetting					kLogChangeSetEditorID;
		}
	}
}