#pragma once

namespace cse
{
	namespace uiManager
	{
		class FaceGenWindowData : public bgsee::WindowExtraData
		{
		public:
			bool				TunnelingTabSelectMessage;
			bool				AllowPreviewUpdates;
			std::string			VoicePlaybackFilePath;

			FaceGenWindowData();
			virtual ~FaceGenWindowData();

			enum { kTypeID = 'XFGD' };
		};

		struct FaceGenVoicePreviewData
		{
			char				VoicePath[MAX_PATH];
			char				LipPath[MAX_PATH];
			UInt32				DelayTime;
		};

		class LoadPluginsWindowData : public bgsee::WindowExtraData
		{
		public:
			enum { kTypeID = 'XLPG' };

			struct PluginFlagCache
			{
				TESFile*	Plugin;
				bool		Loaded;
				bool		Active;

				PluginFlagCache(TESFile* Plugin)
				{
					this->Plugin = Plugin;
					this->Loaded = Plugin->IsLoaded();
					this->Active = Plugin->IsActive();
				}
			};

			std::vector<PluginFlagCache>	PluginFlagsInitialState;

			LoadPluginsWindowData() : bgsee::WindowExtraData(kTypeID) {}
			virtual ~LoadPluginsWindowData() = default;

			void	BuildPluginFlagCache();
			void	UpdatePluginFlagsFromCache();
		};

		std::string AiPackagesWindowFormListGetColumnText(HWND FormList, TESForm* Form, UInt32 ColumnIndex);

		LRESULT CALLBACK FindTextDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK DataDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK ResponseEditorDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK LandscapeTextureUseDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK FilteredDialogQuestDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK AboutDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK RaceDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK CommonDialogExtraFittingsSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK SelectTopicsQuestsSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK LandscapeEditDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK AIPackagesDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK AIFormDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK FaceGenDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK MagicItemFormDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK LeveledItemFormDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK TESObjectCELLDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);

		class DialogExtraFittingsData : public bgsee::WindowExtraData
		{
		public:
			POINT		LastCursorPos;
			HWND		LastCursorPosWindow;
			bool		QuickViewTriggered;

			HWND		AssetControlToolTip;
			TOOLINFO	AssetControlToolData;
			HWND		LastTrackedTool;
			bool		TrackingToolTip;

			DialogExtraFittingsData();
			virtual ~DialogExtraFittingsData();

			enum { kTypeID = 'XDEF' };
		};

		class TESFormEditData : public bgsee::WindowExtraData
		{
		public:
			TESForm*						Buffer;		// stores a temp copy of the form being edited

			TESFormEditData();
			virtual ~TESFormEditData();

			enum { kTypeID = 'XFED' };

			void							FillBuffer(TESForm* Parent);
			bool							HasChanges(TESForm* Parent);
		};

		class TESFormIDListViewData : public bgsee::WindowExtraData
		{
		public:
			bool				DisableDragHandling;

			TESFormIDListViewData();
			virtual ~TESFormIDListViewData();

			enum { kTypeID = 'XFLV' };
		};

		LRESULT CALLBACK CommonDialogExtraFittingsSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK TESFormIDListViewDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK TESFormEditDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);
		LRESULT CALLBACK WindowPosDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams);

		void InitializeMiscWindowOverrides();
	}
}

// custom window messages
// wParam = CSEFaceGenVoicePreviewData*
#define WM_FACEGENPREVIEW_PLAYVOICE				(WM_USER + 2020)

// custom control IDs, as baked into the dialog templates
#define IDC_CSE_DATA_SETSTARTUPPLUGIN           9906
#define IDC_CSE_DATA_LOADSTARTUPPLUGIN          9907
#define IDC_CSE_DATA_SELECTLOADORDER		    9908
#define IDC_CSE_DATA_SELECTNONE					9909
#define IDC_CSE_DATA_FILTEREDIT					9910
#define IDC_CSE_DATA_FILTERNAME					9911
#define IDC_CSE_DATA_FILTERAUTHOR				9912
#define IDC_CSE_DATA_FILTERSUMMARY				9913

#define IDC_CSE_RACE_COPYHAIR                   9915
#define IDC_CSE_RACE_COPYEYES                   9916

#define IDC_CSE_QUEST_EDITRESULTSCRIPT			9928

#define IDC_CSE_RESPONSEWINDOW_FACEGENPREVIEW	9934
// also used in the NPC edit dialog
#define IDC_CSE_RESPONSEWINDOW_VOICEDELAY		9935
