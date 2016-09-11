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

			Depot.push_back(&general::kShowNumericEditorIDWarning);
			Depot.push_back(&general::kCrashHandlerMode);
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

			Depot.push_back(&renderWindowOSD::kShowMouseRef);
			Depot.push_back(&renderWindowOSD::kMouseRefCtrlModified);
			Depot.push_back(&renderWindowOSD::kColorSelectionStats);
			Depot.push_back(&renderWindowOSD::kColorRAMUsage);

			Depot.push_back(&renderWindowOSD::kShowInfoOverlay);
			Depot.push_back(&renderWindowOSD::kShowCellLists);
			Depot.push_back(&renderWindowOSD::kShowSelectionControls);
			Depot.push_back(&renderWindowOSD::kShowToolbar);
			Depot.push_back(&renderWindowOSD::kShowNotifications);
			Depot.push_back(&renderWindowOSD::kShowActiveRefCollections);
			Depot.push_back(&renderWindowOSD::kWindowBGAlpha);
			Depot.push_back(&renderWindowOSD::kFontSize);
			Depot.push_back(&renderWindowOSD::kFontFace);


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
		}

		namespace general
		{
#define GENERAL_INISECTION					"General"

			INISetting		kShowNumericEditorIDWarning("ShowNumericEditorIDWarning", GENERAL_INISECTION, "Displays a warning when editorIDs start with an integer", (SInt32)1);
			INISetting		kCrashHandlerMode("CrashHandlerMode", GENERAL_INISECTION, "0 - Terminate | 1 - Resume | 2 - Ask every time", (SInt32)2);
			INISetting		kFaceGenPreviewResponseWindow("FaceGenPreviewResponseWindow", GENERAL_INISECTION, "Preview voice/lip file playback in NPC dialogs", (SInt32)0);
			INISetting		kFaceGenPreviewVoiceDelay("FaceGenPreviewVoiceDelay", GENERAL_INISECTION, "Delay, in milliseconds, between voice file and lip sync file playback during previews", (SInt32)50);
			INISetting		kDisableSplashScreen("DisableSplashScreen", GENERAL_INISECTION, "Disables the startup splash screen", (SInt32)0);
			INISetting		kShowHallOfFameMembersInTitleBar("ShowHallOfFameMembersInTitleBar", GENERAL_INISECTION, "Displays names of various ES Modding community members in dialog title bars. 0 - Disable | 1 - Object Windows + Preview Windows | 2 - Object Windows + Preview Windows + Edit Dialogs", (SInt32)1);
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
			INISetting		kFixedCameraPivotMul("FixedCameraPivotMul", RENDERER_INISECTION, "Distance multiplier between the fixed pivot and the camera positon", (float)750.f);
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
		}

		namespace renderWindowOSD
		{
#define RWOSD_INISECTION				"RenderWindowOSD"

			INISetting		kShowMouseRef("ShowMouseRef", RWOSD_INISECTION, "Displays details about the reference under the mouse pointer", (SInt32)1);
			INISetting		kMouseRefCtrlModified("MouseRefCtrlModified", RWOSD_INISECTION, "The control key must be held down to display the mouse ref's details", (SInt32)0);

			INISetting		kShowInfoOverlay("ShowInfoOverlay", RWOSD_INISECTION, "Displays details about the current cell, selection, etc on the top-left corner", (SInt32)1);
			INISetting		kShowCellLists("ShowCellLists", RWOSD_INISECTION, "Displays the cell bookmars/recently visited list", (SInt32)1);
			INISetting		kShowSelectionControls("ShowSelectionControls", RWOSD_INISECTION, "Displays the selection edit controls", (SInt32)1);
			INISetting		kShowToolbar("ShowToolbar", RWOSD_INISECTION, "Displays the toolbar", (SInt32)1);
			INISetting		kShowNotifications("ShowNotifications", RWOSD_INISECTION, "Displays notifications", (SInt32)1);
			INISetting		kShowActiveRefCollections("ShowActiveRefCollections", RWOSD_INISECTION, "Displays the reference collections list", (SInt32)1);

			INISetting		kColorSelectionStats("ColorSelectionStats", RWOSD_INISECTION, "Foreground color", "189,237,99");
			INISetting		kColorRAMUsage("ColorRAMUsage", RWOSD_INISECTION, "Foreground color", "230,230,0");
			INISetting		kWindowBGAlpha("WindowBGAlpha", RWOSD_INISECTION, "Window background alpha (0-1)", (float)0.7f);

			INISetting		kFontSize("FontSize", RWOSD_INISECTION, "Font size", (SInt32)16);
			INISetting		kFontFace("FontFace", RWOSD_INISECTION, "Font face. Must correspond to a TTF font file in the Data\\Font directory", "OpenSans-Bold.ttf");
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
		}
	}
}