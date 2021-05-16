#include "Settings.h"

namespace cse
{
	namespace settings
	{
		void Register( bgsee::INISettingDepotT& Depot )
		{
			Depot.push_back(&dialogs::kRenderWindowState);
			Depot.push_back(&dialogs::kObjectWindowState);
			Depot.push_back(&dialogs::kCellViewWindowState);
			Depot.push_back(&dialogs::kSortFormListsByActiveForm);
			Depot.push_back(&dialogs::kColorizeActiveForms);
			Depot.push_back(&dialogs::kActiveFormForeColor);
			Depot.push_back(&dialogs::kActiveFormBackColor);
			Depot.push_back(&dialogs::kShowMainWindowsInTaskbar);
			Depot.push_back(&dialogs::kShowEditDialogsInTaskbar);
			Depot.push_back(&dialogs::kColorizeFormOverrides);
			Depot.push_back(&dialogs::kFormOverrideLevel0ForeColor);
			Depot.push_back(&dialogs::kFormOverrideLevel0BackColor);
			Depot.push_back(&dialogs::kFormOverrideLevel1ForeColor);
			Depot.push_back(&dialogs::kFormOverrideLevel1BackColor);
			Depot.push_back(&dialogs::kFormOverrideLevel2ForeColor);
			Depot.push_back(&dialogs::kFormOverrideLevel2BackColor);
			Depot.push_back(&dialogs::kFormOverrideLevel3ForeColor);
			Depot.push_back(&dialogs::kFormOverrideLevel3BackColor);
			Depot.push_back(&dialogs::kMultiplePreviewWindows);
			Depot.push_back(&dialogs::kPreserveEditorDialogLocations);
			Depot.push_back(&dialogs::kCellViewActiveCellForeColor);
			Depot.push_back(&dialogs::kCellViewActiveCellBackColor);
			Depot.push_back(&dialogs::kCellViewLoadedGridCellForeColor);
			Depot.push_back(&dialogs::kCellViewLoadedGridCellBackColor);

			Depot.push_back(&general::kShowNumericEditorIDWarning);
			Depot.push_back(&general::kCrashHandlerMode);
			Depot.push_back(&general::kSalvageActivePluginOnCrash);
			Depot.push_back(&general::kFaceGenPreviewResponseWindow);
			Depot.push_back(&general::kFaceGenPreviewVoiceDelay);
			Depot.push_back(&general::kDisableSplashScreen);
			Depot.push_back(&general::kShowHallOfFameMembersInTitleBar);

			Depot.push_back(&lod::kPartialTextureResolution);
			Depot.push_back(&lod::kDeletePartialsAfterGeneration);

			Depot.push_back(&plugins::kPreventTimeStampChanges);
			Depot.push_back(&plugins::kSaveLoadedESPsAsMasters);

			Depot.push_back(&renderer::kAltRefMovementSpeed);
			Depot.push_back(&renderer::kAltRefSnapGrid);
			Depot.push_back(&renderer::kAltRefRotationSpeed);
			Depot.push_back(&renderer::kAltRefSnapAngle);
			Depot.push_back(&renderer::kAltCamRotationSpeed);
			Depot.push_back(&renderer::kAltCamZoomSpeed);
			Depot.push_back(&renderer::kAltCamPanSpeed);
			Depot.push_back(&renderer::kCoplanarRefDrops);
			Depot.push_back(&renderer::kFixedCameraPivot);
			Depot.push_back(&renderer::kFixedCameraPivotMul);
			Depot.push_back(&renderer::kCameraFOV);
			Depot.push_back(&renderer::kExteriorSnapshotResolution);
			Depot.push_back(&renderer::kZOffsetDuplicatedRefs);
			Depot.push_back(&renderer::kParentChildVisualIndicator);
			Depot.push_back(&renderer::kRefToggleOpacityAlpha);
			Depot.push_back(&renderer::kPathGridLinkedRefIndicator);
			Depot.push_back(&renderer::kPathGridLinkedRefIndicatorFlags);
			Depot.push_back(&renderer::kGrassOverlayTexturePath);
			Depot.push_back(&renderer::kNotifyOnInvalidExecutionContext);
			Depot.push_back(&renderer::kUnrestrictedMouseMovement);
			Depot.push_back(&renderer::kShowSelectionMask);
			Depot.push_back(&renderer::kSelectionMaskColor);
			Depot.push_back(&renderer::kShowMouseOverMask);
			Depot.push_back(&renderer::kMouseOverMaskColor);

			Depot.push_back(&renderWindowOSD::kShowMouseRef);
			Depot.push_back(&renderWindowOSD::kMouseRefCtrlModified);

			Depot.push_back(&renderWindowOSD::kShowInfoOverlay);
			Depot.push_back(&renderWindowOSD::kShowRefBatchEditor);
			Depot.push_back(&renderWindowOSD::kShowToolbar);
			Depot.push_back(&renderWindowOSD::kShowNotifications);
			Depot.push_back(&renderWindowOSD::kFontSize);
			Depot.push_back(&renderWindowOSD::kFontFace);

			Depot.push_back(&renderWindowOSD::kColorText);
			Depot.push_back(&renderWindowOSD::kColorTextDisabled);
			Depot.push_back(&renderWindowOSD::kColorWindowBg);
			Depot.push_back(&renderWindowOSD::kColorChildBg);
			Depot.push_back(&renderWindowOSD::kColorPopupBg);
			Depot.push_back(&renderWindowOSD::kColorBorder);
			Depot.push_back(&renderWindowOSD::kColorBorderShadow);
			Depot.push_back(&renderWindowOSD::kColorFrameBg);
			Depot.push_back(&renderWindowOSD::kColorFrameBgHovered);
			Depot.push_back(&renderWindowOSD::kColorFrameBgActive);
			Depot.push_back(&renderWindowOSD::kColorTitleBg);
			Depot.push_back(&renderWindowOSD::kColorTitleBgActive);
			Depot.push_back(&renderWindowOSD::kColorTitleBgCollapsed);
			Depot.push_back(&renderWindowOSD::kColorMenuBarBg);
			Depot.push_back(&renderWindowOSD::kColorScrollbarBg);
			Depot.push_back(&renderWindowOSD::kColorScrollbarGrab);
			Depot.push_back(&renderWindowOSD::kColorScrollbarGrabHovered);
			Depot.push_back(&renderWindowOSD::kColorScrollbarGrabActive);
			Depot.push_back(&renderWindowOSD::kColorCheckMark);
			Depot.push_back(&renderWindowOSD::kColorSliderGrab);
			Depot.push_back(&renderWindowOSD::kColorSliderGrabActive);
			Depot.push_back(&renderWindowOSD::kColorButton);
			Depot.push_back(&renderWindowOSD::kColorButtonHovered);
			Depot.push_back(&renderWindowOSD::kColorButtonActive);
			Depot.push_back(&renderWindowOSD::kColorHeader);
			Depot.push_back(&renderWindowOSD::kColorHeaderHovered);
			Depot.push_back(&renderWindowOSD::kColorHeaderActive);
			Depot.push_back(&renderWindowOSD::kColorSeparator);
			Depot.push_back(&renderWindowOSD::kColorSeparatorHovered);
			Depot.push_back(&renderWindowOSD::kColorSeparatorActive);
			Depot.push_back(&renderWindowOSD::kColorResizeGrip);
			Depot.push_back(&renderWindowOSD::kColorResizeGripHovered);
			Depot.push_back(&renderWindowOSD::kColorResizeGripActive);
			Depot.push_back(&renderWindowOSD::kColorTab);
			Depot.push_back(&renderWindowOSD::kColorTabHovered);
			Depot.push_back(&renderWindowOSD::kColorTabActive);
			Depot.push_back(&renderWindowOSD::kColorTabUnfocused);
			Depot.push_back(&renderWindowOSD::kColorTabUnfocusedActive);
			Depot.push_back(&renderWindowOSD::kColorPlotLines);
			Depot.push_back(&renderWindowOSD::kColorPlotLinesHovered);
			Depot.push_back(&renderWindowOSD::kColorPlotHistogram);
			Depot.push_back(&renderWindowOSD::kColorPlotHistogramHovered);
			Depot.push_back(&renderWindowOSD::kColorTableHeaderBg);
			Depot.push_back(&renderWindowOSD::kColorTableBorderStrong);
			Depot.push_back(&renderWindowOSD::kColorTableBorderLight);
			Depot.push_back(&renderWindowOSD::kColorTableRowBg);
			Depot.push_back(&renderWindowOSD::kColorTableRowBgAlt);
			Depot.push_back(&renderWindowOSD::kColorTextSelectedBg);
			Depot.push_back(&renderWindowOSD::kColorDragDropTarget);
			Depot.push_back(&renderWindowOSD::kColorNavHighlight);
			Depot.push_back(&renderWindowOSD::kColorNavWindowingHighlight);
			Depot.push_back(&renderWindowOSD::kColorNavWindowingDimBg);
			Depot.push_back(&renderWindowOSD::kColorModalWindowDimBg);

			Depot.push_back(&renderWindowFlyCamera::kMovementSpeed);
			Depot.push_back(&renderWindowFlyCamera::kSprintMultiplier);
			Depot.push_back(&renderWindowFlyCamera::kCrawlMultiplier);
			Depot.push_back(&renderWindowFlyCamera::kRotationSpeed);
			Depot.push_back(&renderWindowFlyCamera::kCameraFOV);

			Depot.push_back(&startup::kLoadPlugin);
			Depot.push_back(&startup::kPluginName);
			Depot.push_back(&startup::kOpenScriptWindow);
			Depot.push_back(&startup::kScriptEditorID);
			Depot.push_back(&startup::kSetWorkspace);
			Depot.push_back(&startup::kWorkspacePath);

			Depot.push_back(&versionControl::kBackupOnSave);
			Depot.push_back(&versionControl::kLogInstantiation);
			Depot.push_back(&versionControl::kLogChangeSetActive);
			Depot.push_back(&versionControl::kLogChangeSetDeleted);
			Depot.push_back(&versionControl::kLogChangeSetFormID);
			Depot.push_back(&versionControl::kLogChangeSetEditorID);
		}

		namespace dialogs
		{
#define DIALOGS_INISECTION					"Dialogs"

			INISetting		kRenderWindowState("RenderWindowState", DIALOGS_INISECTION, "Window visibility", (SInt32)1);
			INISetting		kObjectWindowState("ObjectWindowState", DIALOGS_INISECTION, "Window visibility", (SInt32)1);
			INISetting		kCellViewWindowState("CellViewWindowState", DIALOGS_INISECTION, "Window visibility", (SInt32)1);

			INISetting		kSortFormListsByActiveForm("SortFormListsByActiveForm", DIALOGS_INISECTION, "Sort active forms first in list views", (SInt32)1);

			INISetting		kColorizeActiveForms("ColorizeActiveForms", DIALOGS_INISECTION, "Colorize active forms in list views", (SInt32)1);
			INISetting		kActiveFormForeColor("ActiveFormForeColor", DIALOGS_INISECTION, "Foreground color of active form items (RGB)", "255,255,255");
			INISetting		kActiveFormBackColor("ActiveFormBackColor", DIALOGS_INISECTION, "Background color of active form items (RGB)", "0,128,64");

			INISetting		kShowMainWindowsInTaskbar("ShowMainWindowsInTaskbar", DIALOGS_INISECTION, "Show the primary CS windows in the taskbar", (SInt32)0);
			INISetting		kShowEditDialogsInTaskbar("ShowEditDialogsInTaskbar", DIALOGS_INISECTION, "Show form edit dialogs in the taskbar", (SInt32)0);

			INISetting		kColorizeFormOverrides("ColorizeFormOverrides", DIALOGS_INISECTION, "Colorize overridden forms in list views", (SInt32)0);
			INISetting		kFormOverrideLevel0ForeColor("FormOverrideLevel0ForeColor", DIALOGS_INISECTION, "Foreground color of overridden form items (RGB)", "255,255,255");
			INISetting		kFormOverrideLevel0BackColor("FormOverrideLevel0BackColor", DIALOGS_INISECTION, "Background color of overridden form items (RGB)", "0,0,0");
			INISetting		kFormOverrideLevel1ForeColor("FormOverrideLevel1ForeColor", DIALOGS_INISECTION, "Foreground color of overridden form items (RGB)", "255,255,255");
			INISetting		kFormOverrideLevel1BackColor("FormOverrideLevel1BackColor", DIALOGS_INISECTION, "Background color of overridden form items (RGB)", "0,0,0");
			INISetting		kFormOverrideLevel2ForeColor("FormOverrideLevel2ForeColor", DIALOGS_INISECTION, "Foreground color of overridden form items (RGB)", "255,255,255");
			INISetting		kFormOverrideLevel2BackColor("FormOverrideLevel2BackColor", DIALOGS_INISECTION, "Background color of overridden form items (RGB)", "0,0,0");
			INISetting		kFormOverrideLevel3ForeColor("FormOverrideLevel3ForeColor", DIALOGS_INISECTION, "Foreground color of overridden form items (RGB)", "255,255,255");
			INISetting		kFormOverrideLevel3BackColor("FormOverrideLevel3BackColor", DIALOGS_INISECTION, "Background color of overridden form items (RGB)", "0,0,0");

			INISetting		kMultiplePreviewWindows("MultiplePreviewWindows", DIALOGS_INISECTION, "Allow multiple preview windows", (SInt32)1);
			INISetting		kPreserveEditorDialogLocations("PreserveEditorDialogLocations", DIALOGS_INISECTION, "Save/Load the desktop locations of editor dialogs to/from the INI file", (SInt32)1);

			INISetting		kCellViewActiveCellForeColor("CellViewActiveCellForeColor", DIALOGS_INISECTION, "Foreground color of the active interior/grid-center exterior cell in the cell view window (RGB)", "0,0,0");
			INISetting		kCellViewActiveCellBackColor("CellViewActiveCellBackColor", DIALOGS_INISECTION, "Background color of the active interior/grid-center exterior cell in the cell view window (RGB)", "102,196,251");

			INISetting		kCellViewLoadedGridCellForeColor("CellViewLoadedGridCellForeColor", DIALOGS_INISECTION, "Foreground color of the non-grid-center exterior cells in the cell view window (RGB)", "200,200,200");
			INISetting		kCellViewLoadedGridCellBackColor("CellViewLoadedGridCellBackColor", DIALOGS_INISECTION, "Background color of the non-grid-center exterior cell in the cell view window (RGB)", "3,89,141");
		}

		namespace general
		{
#define GENERAL_INISECTION					"General"

			INISetting		kShowNumericEditorIDWarning("ShowNumericEditorIDWarning", GENERAL_INISECTION, "Displays a warning when editorIDs start with an integer", (SInt32)1);
			INISetting		kCrashHandlerMode("CrashHandlerMode", GENERAL_INISECTION, "0 - Terminate | 1 - Resume | 2 - Ask every time", (SInt32)2);
			INISetting		kSalvageActivePluginOnCrash("SalvageActivePluginOnCrash", GENERAL_INISECTION, "Attempt to dump the active plugin when the editor crashes", (SInt32)1);
			INISetting		kFaceGenPreviewResponseWindow("FaceGenPreviewResponseWindow", GENERAL_INISECTION, "Preview voice/lip file playback in NPC dialogs", (SInt32)0);
			INISetting		kFaceGenPreviewVoiceDelay("FaceGenPreviewVoiceDelay", GENERAL_INISECTION, "Delay, in milliseconds, between voice file and lip sync file playback during previews", (SInt32)50);
			INISetting		kDisableSplashScreen("DisableSplashScreen", GENERAL_INISECTION, "Disables the startup splash screen", (SInt32)0);
			INISetting		kShowHallOfFameMembersInTitleBar("ShowHallOfFameMembersInTitleBar", GENERAL_INISECTION, "Displays names of various ES modding community members in dialog title bars. 0 - Disable | 1 - Object Windows + Preview Windows | 2 - Object Windows + Preview Windows + Edit Dialogs", (SInt32)1);
		}

		namespace lod
		{
#define LOD_INISECTION						"LOD"

			INISetting		kPartialTextureResolution("PartialTextureResolution", LOD_INISECTION, "Resolution of the generated partial LOD diffuse maps. Size of the full map = 16 * Res. Must be a power of 2 and b'ween 32 and 384", (SInt32)384);
			INISetting		kDeletePartialsAfterGeneration("DeletePartialsAfterGeneration", LOD_INISECTION, "Delete the partial LOD textures after the full map is generated", (SInt32)1);
		}

		namespace plugins
		{
#define PLUGINS_INISECTION					"Plugins"

			INISetting		kPreventTimeStampChanges("PreventTimeStampChanges", PLUGINS_INISECTION, "Prevents modifications to the timestamps of plugins being saved", (SInt32)0);
			INISetting		kSaveLoadedESPsAsMasters("SaveLoadedESPsAsMasters", PLUGINS_INISECTION, "Allows ESP files to be saved as the active plugin's master", (SInt32)1);
		}

		namespace renderer
		{
#define RENDERER_INISECTION					"Renderer"

			INISetting		kAltRefMovementSpeed("AltRefMovementSpeed", RENDERER_INISECTION, "Alternate render window reference movement speed", (float)0.8);
			INISetting		kAltRefSnapGrid("AltRefSnapGrid", RENDERER_INISECTION, "Alternate render window reference snap to grid", (float)2);
			INISetting		kAltRefRotationSpeed("AltRefRotationSpeed", RENDERER_INISECTION, "Alternate render window reference rotation speed", (float)1.0);
			INISetting		kAltRefSnapAngle("AltRefSnapAngle", RENDERER_INISECTION, "Alternate render window reference snap to angle", (float)45);
			INISetting		kAltCamRotationSpeed("AltCamRotationSpeed", RENDERER_INISECTION, "Alternate render window camera rotation speed", (float)1.0);
			INISetting		kAltCamZoomSpeed("AltCamZoomSpeed", RENDERER_INISECTION, "Alternate render window camera zoom speed", (float)0.5);
			INISetting		kAltCamPanSpeed("AltCamPanSpeed", RENDERER_INISECTION, "Alternate render window camera pan speed", (float)5.0);
			INISetting		kCoplanarRefDrops("CoplanarRefDrops", RENDERER_INISECTION, "Place new references co-planar with the object at the cursor location", (SInt32)1);
			INISetting		kFixedCameraPivot("FixedCameraPivot", RENDERER_INISECTION, "Use a static pivot when rotating the viewport camera", (SInt32)0);
			INISetting		kFixedCameraPivotMul("FixedCameraPivotMul", RENDERER_INISECTION, "Distance multiplier between the fixed pivot and the camera position", (float)750.f);
			INISetting		kCameraFOV("CameraFOV", RENDERER_INISECTION, "Default camera field of vision", (float)90);
			INISetting		kExteriorSnapshotResolution("ExteriorSnapshotResolution", RENDERER_INISECTION, "Resolution of the exterior cell snapshot texture. Size must be a power of 2", (SInt32)1024);
			INISetting		kZOffsetDuplicatedRefs("ZOffsetDuplicatedRefs", RENDERER_INISECTION, "Moves duplicated refs slightly in the positive Z axis after creation", (SInt32)0);
			INISetting		kParentChildVisualIndicator("ParentChildVisualIndicator", RENDERER_INISECTION, "Display connectors between enable-state linked references", (SInt32)0);
			INISetting		kRefToggleOpacityAlpha("RefToggleOpacityAlpha", RENDERER_INISECTION, "Reference transparency lower-bound (0-1)", (float)0.4);
			INISetting		kPathGridLinkedRefIndicator("PathGridLinkedRefIndicator", RENDERER_INISECTION, "Display connectors between path grid points and their linked references", (SInt32)1);
			INISetting		kPathGridLinkedRefIndicatorFlags("PathGridLinkedRefIndicatorFlags", RENDERER_INISECTION, "Indicator visibility flags",
							(UInt32)kPathGridLinkedRefIndicatorFlag_HidePointBoundingBox | kPathGridLinkedRefIndicatorFlag_HideLinkedRefNode | kPathGridLinkedRefIndicatorFlag_HideLineConnector);
			INISetting		kGrassOverlayTexturePath("GrassOverlayTexturePath", RENDERER_INISECTION, "Path relative to the Textures\\Landscape folder that gets applied when the grass overlay is enabled", "CSE_GrassOverlay.dds");
			INISetting		kNotifyOnInvalidExecutionContext("NotifyOnInvalidExecutionContext", RENDERER_INISECTION, "Display a message when an operation is invoked in the wrong edit mode", (SInt32)0);
			INISetting		kUnrestrictedMouseMovement("UnrestrictedMouseMovement", RENDERER_INISECTION, "Unrestricted mouse movement when transforming the camera/selection", (SInt32)0);
			INISetting		kShowSelectionMask("ShowSelectionMask", RENDERER_INISECTION, "Apply a color mask on references selected in the render window", (SInt32)1);
			INISetting		kSelectionMaskColor("SelectionMaskColor", RENDERER_INISECTION, "Selection mask color (RGB)", "60,221,5");
			INISetting		kShowMouseOverMask("ShowMouseOverMask", RENDERER_INISECTION, "Apply a color mask on the reference under the mouse cursor in the render window", (SInt32)0);
			INISetting		kMouseOverMaskColor("MouseOverMaskColor", RENDERER_INISECTION, "Mouse over mask color (RGB)", "160,221,5");
		}

		namespace renderWindowOSD
		{
#define RWOSD_INISECTION				"RenderWindowOSD"

			INISetting		kShowMouseRef("ShowMouseRef", RWOSD_INISECTION, "Displays details about the reference or path-grid point under the mouse pointer", (SInt32)1);
			INISetting		kMouseRefCtrlModified("MouseRefCtrlModified", RWOSD_INISECTION, "The control key must be held down to display the mouse ref's details", (SInt32)0);

			INISetting		kShowInfoOverlay("ShowInfoOverlay", RWOSD_INISECTION, "Displays details about the current cell, camera position, etc on the top-left corner", (SInt32)1);
			INISetting		kShowRefBatchEditor("ShowSelectionControls", RWOSD_INISECTION, "Displays the reference batch editor", (SInt32)1);
			INISetting		kShowToolbar("ShowToolbar", RWOSD_INISECTION, "Displays the toolbar", (SInt32)1);
			INISetting		kShowNotifications("ShowNotifications", RWOSD_INISECTION, "Displays notifications", (SInt32)1);

			INISetting		kFontSize("FontSize", RWOSD_INISECTION, "Font size", (SInt32)16);
			INISetting		kFontFace("FontFace", RWOSD_INISECTION, "Font face. Must correspond to a TTF font file in the Data\\Font directory", "Ruda-Bold.ttf");

			INISetting		kColorText("ColorText", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.95,0.96,0.98,1.00");
			INISetting		kColorTextDisabled("ColorTextDisabled", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.36,0.42,0.47,1.00");
			INISetting		kColorWindowBg("ColorWindowBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.11,0.15,0.17,1.00");
			INISetting		kColorChildBg("ColorChildBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.11,0.15,0.17,0.00");
			INISetting		kColorPopupBg("ColorPopupBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.11,0.15,0.17,1.00");
			INISetting		kColorBorder("ColorBorder", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.08,0.10,0.12,1.00");
			INISetting		kColorBorderShadow("ColorBorderShadow", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.00,0.00,0.00,0.00");
			INISetting		kColorFrameBg("ColorFrameBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.20,0.25,0.29,1.00");
			INISetting		kColorFrameBgHovered("ColorFrameBgHovered", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.12,0.20,0.28,1.00");
			INISetting		kColorFrameBgActive("ColorFrameBgActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.09,0.12,0.14,1.00");
			INISetting		kColorTitleBg("ColorTitleBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.09,0.12,0.14,0.65");
			INISetting		kColorTitleBgActive("ColorTitleBgActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.08,0.10,0.12,1.00");
			INISetting		kColorTitleBgCollapsed("ColorTitleBgCollapsed", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.00,0.00,0.00,0.51");
			INISetting		kColorMenuBarBg("ColorMenuBarBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.15,0.18,0.22,1.00");
			INISetting		kColorScrollbarBg("ColorScrollbarBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.02,0.02,0.02,0.39");
			INISetting		kColorScrollbarGrab("ColorScrollbarGrab", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.20,0.25,0.29,1.00");
			INISetting		kColorScrollbarGrabHovered("ColorScrollbarGrabHovered", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.18,0.22,0.25,1.00");
			INISetting		kColorScrollbarGrabActive("ColorScrollbarGrabActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.09,0.21,0.31,1.00");
			INISetting		kColorCheckMark("ColorCheckMark", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.28,0.56,1.00,1.00");
			INISetting		kColorSliderGrab("ColorSliderGrab", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.28,0.56,1.00,1.00");
			INISetting		kColorSliderGrabActive("ColorSliderGrabActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.37,0.61,1.00,1.00");
			INISetting		kColorButton("ColorButton", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.20,0.25,0.29,1.00");
			INISetting		kColorButtonHovered("ColorButtonHovered", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.28,0.56,1.00,1.00");
			INISetting		kColorButtonActive("ColorButtonActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.06,0.53,0.98,1.00");
			INISetting		kColorHeader("ColorHeader", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.20,0.25,0.29,0.55");
			INISetting		kColorHeaderHovered("ColorHeaderHovered", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.26,0.59,0.98,0.80");
			INISetting		kColorHeaderActive("ColorHeaderActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.26,0.59,0.98,1.00");
			INISetting		kColorSeparator("ColorSeparator", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.20,0.25,0.29,1.00");
			INISetting		kColorSeparatorHovered("ColorSeparatorHovered", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.10,0.40,0.75,0.78");
			INISetting		kColorSeparatorActive("ColorSeparatorActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.10,0.40,0.75,1.00");
			INISetting		kColorResizeGrip("ColorResizeGrip", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.26,0.59,0.98,0.25");
			INISetting		kColorResizeGripHovered("ColorResizeGripHovered", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.26,0.59,0.98,0.67");
			INISetting		kColorResizeGripActive("ColorResizeGripActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.26,0.59,0.98,0.95");
			INISetting		kColorTab("ColorTab", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.11,0.15,0.17,1.00");
			INISetting		kColorTabHovered("ColorTabHovered", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.26,0.59,0.98,0.80");
			INISetting		kColorTabActive("ColorTabActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.20,0.25,0.29,1.00");
			INISetting		kColorTabUnfocused("ColorTabUnfocused", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.11,0.15,0.17,1.00");
			INISetting		kColorTabUnfocusedActive("ColorTabUnfocusedActive", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.11,0.15,0.17,1.00");
			INISetting		kColorPlotLines("ColorPlotLines", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.61,0.61,0.61,1.00");
			INISetting		kColorPlotLinesHovered("ColorPlotLinesHovered", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "1.00,0.43,0.35,1.00");
			INISetting		kColorPlotHistogram("ColorPlotHistogram", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.90,0.70,0.00,1.00");
			INISetting		kColorPlotHistogramHovered("ColorPlotHistogramHovered", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "1.00,0.60,0.00,1.00");
			INISetting		kColorTableHeaderBg("ColorTableHeaderBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.19,0.19,0.20,1.00");
			INISetting		kColorTableBorderStrong("ColorTableBorderStrong", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.31,0.31,0.35,1.00");
			INISetting		kColorTableBorderLight("ColorTableBorderLight", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.23,0.23,0.25,1.00");
			INISetting		kColorTableRowBg("ColorTableRowBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.00,0.00,0.00,0.00");
			INISetting		kColorTableRowBgAlt("ColorTableRowBgAlt", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "1.00,1.00,1.00,0.07");
			INISetting		kColorTextSelectedBg("ColorTextSelectedBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.26,0.59,0.98,0.35");
			INISetting		kColorDragDropTarget("ColorDragDropTarget", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "1.00,1.00,0.00,0.90");
			INISetting		kColorNavHighlight("ColorNavHighlight", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.26,0.59,0.98,1.00");
			INISetting		kColorNavWindowingHighlight("ColorNavWindowingHighlight", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "1.00,1.00,1.00,0.70");
			INISetting		kColorNavWindowingDimBg("ColorNavWindowingDimBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.80,0.80,0.80,0.20");
			INISetting		kColorModalWindowDimBg("ColorModalWindowDimBg", RWOSD_INISECTION, "Color parameter in (normalized) RGBA format", "0.80,0.80,0.80,0.35");
		}

		namespace renderWindowFlyCamera
		{
#define RWFLYCAM_INISECTION					"RenderWindowFlyCamera"

			INISetting		kMovementSpeed("MovementSpeed", RWFLYCAM_INISECTION, "Fly camera movement speed", (float)10);
			INISetting		kSprintMultiplier("SprintMultiplier", RWFLYCAM_INISECTION, "Fly camera sprint speed multiplier", (float)7.5);
			INISetting		kCrawlMultiplier("CrawlMultiplier", RWFLYCAM_INISECTION, "Fly camera crawl speed multiplier", (float)0.05);
			INISetting		kRotationSpeed("RotationSpeed", RWFLYCAM_INISECTION, "Fly camera rotation", (float)1.0);
			INISetting		kCameraFOV("CameraFOV", RWFLYCAM_INISECTION, "Fly camera field of vision", (float)90);
		}

		namespace startup
		{
#define STARTUP_INISECTION					"Startup"

			INISetting		kLoadPlugin("LoadPlugin", STARTUP_INISECTION, "Load a plugin on CS startup", (SInt32)0);
			INISetting		kPluginName("PluginName", STARTUP_INISECTION, "Name of the plugin, with extension, that is to be loaded on startup", "");
			INISetting		kOpenScriptWindow("OpenScriptWindow", STARTUP_INISECTION, "Open an empty script editor window on startup", (SInt32)0);
			INISetting		kScriptEditorID("ScriptEditorID", STARTUP_INISECTION, "EditorID of the script to be loaded on startup, should a script editor also be opened. An empty string results in a blank workspace", "");
			INISetting		kSetWorkspace("SetWorkspace", STARTUP_INISECTION, "Set the working directory to a custom path on startup", (SInt32)0);
			INISetting		kWorkspacePath("WorkspacePath", STARTUP_INISECTION, "Path of the custom workspace directory", "");
		}

		namespace versionControl
		{
#define VERSIONCTRL_INISECTION				"VersionControl"

			INISetting		kBackupOnSave("BackupOnSave", VERSIONCTRL_INISECTION, "Creates a backup copy of the active plugin in the active workspace's 'Backup' directory before commencing a save operation", (SInt32)0);

			INISetting		kLogInstantiation("LogInstantiation", VERSIONCTRL_INISECTION, "Log when a form is created", (SInt32)0);
			INISetting		kLogChangeSetActive("LogChangeSetActive", VERSIONCTRL_INISECTION, "Log when a form's modified state is changed", (SInt32)0);
			INISetting		kLogChangeSetDeleted("LogChangeSetDeleted", VERSIONCTRL_INISECTION, "Log when a form's deleted state is changed", (SInt32)0);
			INISetting		kLogChangeSetFormID("LogChangeSetFormID", VERSIONCTRL_INISECTION, "Log when a form's formID is changed", (SInt32)0);
			INISetting		kLogChangeSetEditorID("LogChangeSetEditorID", VERSIONCTRL_INISECTION, "Log when a form's editorID is changed", (SInt32)0);
		}
	}
}