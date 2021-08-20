#pragma once

namespace cse
{
	namespace uiManager
	{
		BOOL CALLBACK			AssetSelectorDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			TextEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			TESFileSaveDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			TESComboBoxDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK		CreateGlobalScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			BindScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK			EditResultScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	}
}