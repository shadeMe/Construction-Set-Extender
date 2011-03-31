#pragma once

extern WNDPROC						g_FindTextOrgWindowProc;
extern WNDPROC						g_DataDlgOrgWindowProc;
extern WNDPROC						g_CSMainWndOrgWindowProc;
extern WNDPROC						g_RenderWndOrgWindowProc;
extern WNDPROC						g_ConsoleWndOrgWindowProc;
extern WNDPROC						g_ConsoleEditControlOrgWindowProc;
extern WNDPROC						g_ConsoleCmdBoxOrgWindowProc;
extern WNDPROC						g_ObjectWndOrgWindowProc;
extern WNDPROC						g_CellViewWndOrgWindowProc;
extern WNDPROC						g_ResponseWndOrgWindowProc;

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

LRESULT CALLBACK CopyPathMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK ConsoleDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConsoleEditControlSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConsoleCmdBoxSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK BindScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void EvaluatePopupMenuItems(HWND hWnd, int Identifier, TESForm* Form);

LRESULT CALLBACK ObjectWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CellViewWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ResponseWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void InitializeWindowManager();

extern HFONT					g_CSDefaultFont;