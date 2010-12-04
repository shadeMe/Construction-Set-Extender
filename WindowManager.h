#pragma once

extern WNDPROC						g_FindTextOrgWindowProc;
extern WNDPROC						g_DataDlgOrgWindowProc;
extern WNDPROC						g_CSMainWndOrgWindowProc;
extern WNDPROC						g_RenderWndOrgWindowProc;
extern WNDPROC						g_ConsoleWndOrgWindowProc;
extern WNDPROC						g_ConsoleEditControlOrgWindowProc;
extern WNDPROC						g_ConsoleCmdBoxOrgWindowProc;


LRESULT CALLBACK FindTextDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DataDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CSMainWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RenderWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK AssetSelectorDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
enum AssetSelectorResult
{
	e_Close = 0,
	e_FileBrowser,
	e_BSABrowser,
	e_EditPath,
	e_ClearPath
};
BOOL CALLBACK TextEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TESFileDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ConsoleDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConsoleEditControlSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConsoleCmdBoxSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);