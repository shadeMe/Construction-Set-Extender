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


		LRESULT CALLBACK		FindTextDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		DataDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		ResponseDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		LandscapeTextureUseDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		FilteredDialogQuestDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		AboutDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		RaceDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		CommonDialogExtraFittingsSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		SelectTopicsQuestsSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		LandscapeEditDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		AIPackagesDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		AIFormDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		FaceGenDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		MagicItemFormDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		LeveledItemFormDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);
		LRESULT CALLBACK		TESObjectCELLDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData);

		void					InitializeMiscWindowOverrides();
	}
}

// custom window messages
// wParam = CSEFaceGenVoicePreviewData*
#define WM_FACEGENPREVIEW_PLAYVOICE				(WM_USER + 2020)
// lParam = TESFile*
#define WM_DATADLG_RECURSEMASTERS				(WM_USER + 2002)

// custom control IDs, as baked into the dialog templates
#define IDC_CSE_DATA_SETSTARTUPPLUGIN           9906
#define IDC_CSE_DATA_LOADSTARTUPPLUGIN          9907
#define IDC_CSE_DATA_SELECTLOADORDER		    9908
#define IDC_CSE_DATA_SELECTNONE					9909

#define IDC_CSE_RACE_COPYHAIR                   9915
#define IDC_CSE_RACE_COPYEYES                   9916

#define IDC_CSE_QUEST_EDITRESULTSCRIPT			9928

#define IDC_CSE_RESPONSEWINDOW_FACEGENPREVIEW	9934
// also used in the NPC edit dialog
#define IDC_CSE_RESPONSEWINDOW_VOICEDELAY		9935
