#pragma once
#include "WindowEdgeSnapper.h"

extern WNDPROC						g_FindTextOrgWindowProc;
extern WNDPROC						g_DataDlgOrgWindowProc;
extern WNDPROC						g_CSMainWndOrgWindowProc;
extern WNDPROC						g_RenderWndOrgWindowProc;
extern WNDPROC						g_ObjectWndOrgWindowProc;
extern WNDPROC						g_CellViewWndOrgWindowProc;
extern WNDPROC						g_ResponseWndOrgWindowProc;
extern WNDPROC						g_TagBrowserOrgWindowProc;
extern WNDPROC						g_LandscapeTextureUseOrgWindowProc;

LRESULT CALLBACK FindTextDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DataDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CSMainWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RenderWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK AssetSelectorDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TextEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TESFileDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GlobalScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TESComboBoxDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CopyPathDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK BindScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void EvaluatePopupMenuItems(HWND hWnd, int Identifier, TESForm* Form);

LRESULT CALLBACK ObjectWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CellViewWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ResponseWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TagBrowserSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ManageToolsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CSASGlobalsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AchievementUnlockedDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LandscapeTextureUseSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EditResultScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DebugViewBMPDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool TXTChannelStaticHandler_RAMUsage(std::string& RenderedText);

void InitializeWindowManager();

extern HFONT					g_CSDefaultFont;
extern const POINT				g_ObjectWindowTreePosOffset;
extern const POINT				g_CellViewWindowObjListPosOffset;
extern std::string				g_ObjectWindowFilterStr;
extern std::string				g_CellViewWindowFilterStr;
extern CSnapWindow				g_WindowEdgeSnapper;

template <typename T>
struct InitDialogMessageParamT
{
	char						Buffer[0x400];
	T							ExtraData;					
};

// custom control IDs
#define DATA_QUICKLOAD                  9900
#define MAIN_DATA_SAVEAS                9901
#define MAIN_WORLD_BATCHEDIT            9902
#define RENDER_BATCHEDIT                9902
#define MAIN_VIEW_CONSOLEWINDOW         9903
#define MAIN_VIEW_MODIFIEDRECORDS       9904
#define MAIN_DATA_CSEPREFERENCES        9905
#define MAIN_VIEW_DELETEDRECORDS        9906
#define DATA_SETSTARTUPPLUGIN           9906
#define POPUP_SETFORMID                 9907
#define POPUP_MARKUNMODIFIED            9908
#define POPUP_JUMPTOUSEINFOLIST         9909
#define POPUP_UNDELETE                  9910
#define MAIN_WORLD_UNLOADCELL           9911
#define MAIN_GAMEPLAY_GLOBALSCRIPT      9912
#define POPUP_EDITBASEFORM              9913
#define MAIN_LAUNCHGAME                 9914
#define RACE_COPYHAIR                   9915
#define RACE_COPYEYES                   9916
#define POPUP_TOGGLEVISIBILITY          9917
#define POPUP_TOGGLECHILDRENVISIBILITY  9918
#define POPUP_ADDTOTAG                  9919
#define MAIN_VIEW_TAGBROWSER            9920
#define RENDER_GROUPSELECTION           9921
#define RENDER_UNGROUPSELECTION         9922
#define MAIN_DATA_SETWORKSPACE          9923
#define MAIN_DATA_SAVEESPMASTERS        9924
#define POPUP_SHOWOVERRIDES             9925
#define OBJCELL_FILTERLBL				9926
#define OBJCELL_FILTEREDIT				9927
#define QUEST_EDITRESULTSCRIPT			9928
#define CELLVIEW_XLBL					9929
#define CELLVIEW_YLBL					9930
#define CELLVIEW_XEDIT					9931
#define CELLVIEW_YEDIT					9932
#define CELLVIEW_GOBTN					9933
#define MAIN_VIEW_AUXVIEWPORT	        9934
#define PERFCOUNTER_TIMER				9935