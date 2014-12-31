#include "CSESettings.h"

namespace ConstructionSetExtender
{
	namespace Settings
	{
		void Register( BGSEditorExtender::INISettingDepotT& Depot )
		{
			Depot.push_back(&Dialogs::kRenderWindowState);
			Depot.push_back(&Dialogs::kObjectWindowState);
			Depot.push_back(&Dialogs::kCellViewWindowState);
			Depot.push_back(&Dialogs::kSortFormListsByActiveForm);
			Depot.push_back(&Dialogs::kColorizeActiveForms);
			Depot.push_back(&Dialogs::kActiveFormForeColor);
			Depot.push_back(&Dialogs::kActiveFormBackColor);
			Depot.push_back(&Dialogs::kShowMainWindowsInTaskbar);
			Depot.push_back(&Dialogs::kShowEditDialogsInTaskbar);
			Depot.push_back(&Dialogs::kColorizeFormOverrides);
			Depot.push_back(&Dialogs::kFormOverrideLevel0ForeColor);
			Depot.push_back(&Dialogs::kFormOverrideLevel0BackColor);
			Depot.push_back(&Dialogs::kFormOverrideLevel1ForeColor);
			Depot.push_back(&Dialogs::kFormOverrideLevel1BackColor);
			Depot.push_back(&Dialogs::kFormOverrideLevel2ForeColor);
			Depot.push_back(&Dialogs::kFormOverrideLevel2BackColor);
			Depot.push_back(&Dialogs::kFormOverrideLevel3ForeColor);
			Depot.push_back(&Dialogs::kFormOverrideLevel3BackColor);

			Depot.push_back(&General::kShowNumericEditorIDWarning);
			Depot.push_back(&General::kCrashHandlerMode);
			Depot.push_back(&General::kFaceGenPreviewResponseWindow);
			Depot.push_back(&General::kFaceGenPreviewVoiceDelay);
			Depot.push_back(&General::kDisableSplashScreen);
			Depot.push_back(&General::kShowSecondaryHallOfFameMembers);

			Depot.push_back(&LOD::kPartialTextureResolution);
			Depot.push_back(&LOD::kDeletePartialsAfterGeneration);

			Depot.push_back(&Plugins::kPreventTimeStampChanges);
			Depot.push_back(&Plugins::kSaveLoadedESPsAsMasters);

			Depot.push_back(&Renderer::kUpdatePeriod);
			Depot.push_back(&Renderer::kUpdateViewPortAsync);
			Depot.push_back(&Renderer::kAltRefMovementSpeed);
			Depot.push_back(&Renderer::kAltRefSnapGrid);
			Depot.push_back(&Renderer::kAltRefRotationSpeed);
			Depot.push_back(&Renderer::kAltRefSnapAngle);
			Depot.push_back(&Renderer::kAltCamRotationSpeed);
			Depot.push_back(&Renderer::kAltCamZoomSpeed);
			Depot.push_back(&Renderer::kAltCamPanSpeed);
			Depot.push_back(&Renderer::kCoplanarRefDrops);
			Depot.push_back(&Renderer::kSwitchCAndY);
			Depot.push_back(&Renderer::kFixedCameraPivot);
			Depot.push_back(&Renderer::kCameraFOV);
			Depot.push_back(&Renderer::kExteriorSnapshotResolution);
			Depot.push_back(&Renderer::kZOffsetDuplicatedRefs);

			Depot.push_back(&RenderWindowPainter::kShowSelectionStats);
			Depot.push_back(&RenderWindowPainter::kShowRAMUsage);
			Depot.push_back(&RenderWindowPainter::kShowMouseRef);
			Depot.push_back(&RenderWindowPainter::kMouseRefCtrlModified);
			Depot.push_back(&RenderWindowPainter::kFontFace);
			Depot.push_back(&RenderWindowPainter::kFontSize);

			Depot.push_back(&RenderWindowFlyCamera::kMovementSpeed);
			Depot.push_back(&RenderWindowFlyCamera::kSprintMultiplier);
			Depot.push_back(&RenderWindowFlyCamera::kCrawlMultiplier);
			Depot.push_back(&RenderWindowFlyCamera::kRotationSpeed);
			Depot.push_back(&RenderWindowFlyCamera::kCameraFOV);

			Depot.push_back(&Startup::kLoadPlugin);
			Depot.push_back(&Startup::kPluginName);
			Depot.push_back(&Startup::kOpenScriptWindow);
			Depot.push_back(&Startup::kScriptEditorID);
			Depot.push_back(&Startup::kSetWorkspace);
			Depot.push_back(&Startup::kWorkspacePath);

			Depot.push_back(&VersionControl::kBackupOnSave);
		}

		namespace Dialogs
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
		}

		namespace General
		{
#define GENERAL_INISECTION					"General"

			INISetting		kShowNumericEditorIDWarning("ShowNumericEditorIDWarning", GENERAL_INISECTION, "Displays a warning when editorIDs start with an integer", (SInt32)1);
			INISetting		kCrashHandlerMode("CrashHandlerMode", GENERAL_INISECTION, "0 - Terminate | 1 - Resume | 2 - Ask every time", (SInt32)2);
			INISetting		kFaceGenPreviewResponseWindow("FaceGenPreviewResponseWindow", GENERAL_INISECTION, "Preview voice/lip file playback in NPC dialogs", (SInt32)0);
			INISetting		kFaceGenPreviewVoiceDelay("FaceGenPreviewVoiceDelay", GENERAL_INISECTION, "Delay, in milliseconds, between voice file and lip sync file playback during previews", (SInt32)50);
			INISetting		kDisableSplashScreen("DisableSplashScreen", GENERAL_INISECTION, "Disables the startup splash screen", (SInt32)0);
			INISetting		kShowSecondaryHallOfFameMembers("ShowSecondaryHallOfFameMembers", GENERAL_INISECTION, "Displays names of various ES Modding community members in dialog title bars. 0 - Disable | 1 - Object Windows + Preview Windows | 2 - Object Windows + Preview Windows + Edit Dialogs", (SInt32)1);
		}

		namespace LOD
		{
#define LOD_INISECTION						"LOD"

			INISetting		kPartialTextureResolution("PartialTextureResolution", LOD_INISECTION, "Resolution of the generated partial LOD diffuse maps. Size of the full map = 16 * Res. Must be a power of 2 and b'ween 32 and 384", (SInt32)384);
			INISetting		kDeletePartialsAfterGeneration("DeletePartialsAfterGeneration", LOD_INISECTION, "Delete the partial LOD textures after the full map is generated", (SInt32)1);
		}

		namespace Plugins
		{
#define PLUGINS_INISECTION					"Plugins"

			INISetting		kPreventTimeStampChanges("PreventTimeStampChanges", PLUGINS_INISECTION, "Prevents modifications to the timestamps of plugins being saved", (SInt32)0);
			INISetting		kSaveLoadedESPsAsMasters("SaveLoadedESPsAsMasters", PLUGINS_INISECTION, "Allows ESP files to be saved as the active plugin's master", (SInt32)1);
		}

		namespace Renderer
		{
#define RENDERER_INISECTION					"Renderer"

			INISetting		kUpdatePeriod("UpdatePeriod", RENDERER_INISECTION, "Duration, in milliseconds, between render window updates", (SInt32)8);
			INISetting		kUpdateViewPortAsync("UpdateViewPortAsync", RENDERER_INISECTION, "Constantly update the render window in the background", (SInt32)0);
			INISetting		kAltRefMovementSpeed("AltRefMovementSpeed", RENDERER_INISECTION, "Alternate render window reference movement speed", (float)0.8);
			INISetting		kAltRefSnapGrid("AltRefSnapGrid", RENDERER_INISECTION, "Alternate render window reference snap to grid", (float)2);
			INISetting		kAltRefRotationSpeed("AltRefRotationSpeed", RENDERER_INISECTION, "Alternate render window reference rotation speed", (float)1.0);
			INISetting		kAltRefSnapAngle("AltRefSnapAngle", RENDERER_INISECTION, "Alternate render window reference snap to angle", (float)45);
			INISetting		kAltCamRotationSpeed("AltCamRotationSpeed", RENDERER_INISECTION, "Alternate render window camera rotation speed", (float)1.0);
			INISetting		kAltCamZoomSpeed("AltCamZoomSpeed", RENDERER_INISECTION, "Alternate render window camera zoom speed", (float)0.5);
			INISetting		kAltCamPanSpeed("AltCamPanSpeed", RENDERER_INISECTION, "Alternate render window camera pan speed", (float)5.0);
			INISetting		kCoplanarRefDrops("CoplanarRefDrops", RENDERER_INISECTION, "Place new references co-planar with the object at the cursor location", (SInt32)1);
			INISetting		kSwitchCAndY("SwitchCAndY", RENDERER_INISECTION, "Switch the functionalities of the C and Y hotkeys", (SInt32)0);
			INISetting		kFixedCameraPivot("FixedCameraPivot", RENDERER_INISECTION, "Use a static pivot when rotating the viewport camera without a selection", (SInt32)0);
			INISetting		kCameraFOV("CameraFOV", RENDERER_INISECTION, "Default camera field of vision", (float)90);
			INISetting		kExteriorSnapshotResolution("ExteriorSnapshotResolution", RENDERER_INISECTION, "Resolution of the exterior cell snapshot texture. Size must be a power of 2", (SInt32)1024);
			INISetting		kZOffsetDuplicatedRefs("ZOffsetDuplicatedRefs", RENDERER_INISECTION, "Moves duplicated refs slightly in the positive Z axis after creation", (SInt32)0);
		}

		namespace RenderWindowPainter
		{
#define RWPAINTER_INISECTION				"RenderWindowPainter"

			INISetting		kShowSelectionStats("ShowSelectionStats", RWPAINTER_INISECTION, "Displays selection details", (SInt32)1);
			INISetting		kShowRAMUsage("ShowRAMUsage", RWPAINTER_INISECTION, "Displays the editor's RAM usage", (SInt32)1);
			INISetting		kShowMouseRef("ShowMouseRef", RWPAINTER_INISECTION, "Displays details about the reference under the mouse pointer", (SInt32)1);
			INISetting		kMouseRefCtrlModified("MouseRefCtrlModified", RWPAINTER_INISECTION, "The control key must be held down to display the mouse ref's details", (SInt32)0);
			INISetting		kFontFace("FontFace", RWPAINTER_INISECTION, "Font face of the rendered text", "Arial");
			INISetting		kFontSize("FontSize", RWPAINTER_INISECTION, "Size of the rendered text", (SInt32)20);
		}

		namespace RenderWindowFlyCamera
		{
#define RWFLYCAM_INISECTION					"RenderWindowFlyCamera"

			INISetting		kMovementSpeed("MovementSpeed", RWFLYCAM_INISECTION, "Fly camera movement speed", (float)10);
			INISetting		kSprintMultiplier("SprintMultiplier", RWFLYCAM_INISECTION, "Fly camera sprint speed multiplier", (float)7.5);
			INISetting		kCrawlMultiplier("CrawlMultiplier", RWFLYCAM_INISECTION, "Fly camera crawl speed multiplier", (float)0.05);
			INISetting		kRotationSpeed("RotationSpeed", RWFLYCAM_INISECTION, "Fly camera rotation", (float)1.0);
			INISetting		kCameraFOV("CameraFOV", RWFLYCAM_INISECTION, "Fly camera field of vision", (float)90);
		}

		namespace Startup
		{
#define STARTUP_INISECTION					"Startup"

			INISetting		kLoadPlugin("LoadPlugin", STARTUP_INISECTION, "Load a plugin on CS startup", (SInt32)0);
			INISetting		kPluginName("PluginName", STARTUP_INISECTION, "Name of the plugin, with extension, that is to be loaded on startup", "");
			INISetting		kOpenScriptWindow("OpenScriptWindow", STARTUP_INISECTION, "Open an empty script editor window on startup", (SInt32)0);
			INISetting		kScriptEditorID("ScriptEditorID", STARTUP_INISECTION, "EditorID of the script to be loaded on startup, should a script editor also be opened. An empty string results in a blank workspace", "");
			INISetting		kSetWorkspace("SetWorkspace", STARTUP_INISECTION, "Set the working directory to a custom path on startup", (SInt32)0);
			INISetting		kWorkspacePath("WorkspacePath", STARTUP_INISECTION, "Path of the custom workspace directory", "");
		}

		namespace VersionControl
		{
#define VERSIONCTRL_INISECTION				"VersionControl"

			INISetting		kBackupOnSave("BackupOnSave", VERSIONCTRL_INISECTION, "Creates a backup copy of the active plugin in the active workspace's 'Backup' directory before commencing a save operation", (SInt32)0);
		}
	}
}