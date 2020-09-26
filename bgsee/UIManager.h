#pragma once
#include "Main.h"
#include "HookUtil.h"
#include "UICommon.h"
#include "ResourceLocation.h"
#include "WindowSubclasser.h"

namespace bgsee
{
	class UIManager;

	class ResourceTemplateHotSwapper
	{
		friend class UIManager;
	protected:
		typedef std::unordered_map<ResourceTemplateOrdinalT, HINSTANCE>		TemplateResourceInstanceMapT;

		ResourceLocation				SourceDepot;
		TemplateResourceInstanceMapT	TemplateMap;

		void	PopulateTemplateMap(void);
		void	ReleaseTemplateMap(void);
	public:
		ResourceTemplateHotSwapper(std::string SourcePath);
		virtual ~ResourceTemplateHotSwapper() = 0;

		virtual HINSTANCE	GetAlternateResourceInstance(ResourceTemplateOrdinalT TemplateID);		// returns NULL if there isn't one
	};

	class DialogTemplateHotSwapper : public ResourceTemplateHotSwapper
	{
		static const std::string	kDefaultLoc;
	public:
		DialogTemplateHotSwapper();
		virtual ~DialogTemplateHotSwapper();
	};

	class MenuTemplateHotSwapper : public ResourceTemplateHotSwapper
	{
	public:
		MenuTemplateHotSwapper();
		virtual ~MenuTemplateHotSwapper();

		bool	RegisterTemplateReplacer(ResourceTemplateOrdinalT TemplateID, HINSTANCE Replacer);
		bool	UnregisterTemplateReplacer(ResourceTemplateOrdinalT TemplateID);
	};

	class WindowStyler
	{
		friend class UIManager;
	public:
		struct StyleData
		{
			LONG	Regular;
			UInt8	RegularOp;
			LONG	Extended;
			UInt8	ExtendedOp;

			enum
			{
				kOperation_None		= 0,
				kOperation_OR,
				kOperation_AND,
				kOperation_Replace
			};
		};
	private:
		typedef std::unordered_map<ResourceTemplateOrdinalT, StyleData>		TemplateStyleMapT;

		TemplateStyleMapT	StyleListings;

		bool				StyleWindow(HWND Window, ResourceTemplateOrdinalT Template);
	public:
		WindowStyler();
		~WindowStyler();

		bool				RegisterStyle(ResourceTemplateOrdinalT TemplateID, StyleData& Data);
		bool				UnregisterStyle(ResourceTemplateOrdinalT TemplateID);

		void				StyleWindow(HWND Window, StyleData& Data);
	};

	class WindowInvalidationManager
	{
		typedef std::unordered_map<HWND, UInt32>		InvalidationMapT;

		InvalidationMapT	ActiveInvalidatedWindows;

		void				Invalidate(HWND Window, bool State);
	public:
		WindowInvalidationManager();
		~WindowInvalidationManager();

		void	Push(HWND Window);
		void	Pop(HWND Window);
		void	Redraw(HWND Window);
	};

	class UIManager
	{
	public:
		static SME::UIHelpers::CSnapWindow		WindowEdgeSnapper;

		enum
		{
			kHandleCollection_MainWindowChildren	= 0,
			kHandleCollection_DragDropableWindows,

			kHandleCollection__MAX,
		};
	private:
		static UIManager*						Singleton;

		typedef HWND	(CALLBACK* _CallbackCreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
		typedef HMENU	(CALLBACK* _CallbackLoadMenuA)(HINSTANCE hInstance, LPCSTR lpMenuName);
		typedef HWND	(CALLBACK* _CallbackCreateDialogParamA)(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
		typedef INT_PTR	(CALLBACK* _CallbackDialogBoxParamA)(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);

		typedef util::ThunkStdCall<UIManager, HWND, DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID> ThunkCreateWindowExAT;
		typedef util::ThunkStdCall<UIManager, HMENU, HINSTANCE, LPCSTR> ThunkLoadMenuAT;
		typedef util::ThunkStdCall<UIManager, HWND, HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM> ThunkCreateDialogParamAT;
		typedef util::ThunkStdCall<UIManager, INT_PTR, HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM> ThunkDialogParamAT;

		static BOOL CALLBACK					EnumThreadWindowsCallback(HWND hwnd, LPARAM lParam);

		enum
		{
			kIATPatch_CreateWindowEx		= 0,
			kIATPatch_LoadMenu,
			kIATPatch_CreateDialogParam,
			kIATPatch_DialogBoxParam,

			kIATPatch__MAX
		};

		UIManager(const char* MainWindowClassName, HMENU MainMenuHandle);
		~UIManager();

		DWORD						OwnerThreadID;
		util::IATPatchData			PatchDepot[kIATPatch__MAX];
		ThunkCreateWindowExAT		ThunkCreateWindowExA;
		ThunkLoadMenuAT				ThunkLoadMenuA;
		ThunkCreateDialogParamAT	ThunkCreateDialogParamA;
		ThunkDialogParamAT			ThunkDialogParamA;
		std::string					EditorWindowClassName;
		HWND						EditorWindowHandle;
		HINSTANCE					EditorResourceInstance;
		HMENU						EditorMainMenuReplacement;
		WindowSubclasser*			Subclasser;
		DialogTemplateHotSwapper*	DialogHotSwapper;
		MenuTemplateHotSwapper*		MenuHotSwapper;
		WindowStyler*				Styler;
		WindowInvalidationManager*	InvalidationManager;
		WindowHandleCollection		HandleCollections[kHandleCollection__MAX];

		bool						Initialized;

		void		PatchIAT(UInt8 PatchType, void* Callback);
		HWND		CallbackCreateWindowExA(DWORD dwExStyle,
											LPCSTR lpClassName,
											LPCSTR lpWindowName,
											DWORD dwStyle,
											int X, int Y,
											int nWidth, int nHeight,
											HWND hWndParent,
											HMENU hMenu,
											HINSTANCE hInstance,
											LPVOID lpParam);

		HMENU 		CallbackLoadMenuA(HINSTANCE hInstance, LPCSTR lpMenuName);

		HWND		CallbackCreateDialogParamA(HINSTANCE hInstance,
											LPCSTR lpTemplateName,
											HWND hWndParent,
											DLGPROC lpDialogFunc,
											LPARAM dwInitParam);
		HWND		CallbackCreateDialogParamA(HINSTANCE hInstance,
											LPCSTR lpTemplateName,
											HWND hWndParent,
											DLGPROC lpDialogFunc,
											LPARAM dwInitParam,
											const DialogCreationData& CreationData);

		INT_PTR		CallbackDialogBoxParamA(HINSTANCE hInstance,
											LPCSTR lpTemplateName,
											HWND hWndParent,
											DLGPROC lpDialogFunc,
											LPARAM dwInitParam);
		INT_PTR		CallbackDialogBoxParamA(HINSTANCE hInstance,
											LPCSTR lpTemplateName,
											HWND hWndParent,
											DLGPROC lpDialogFunc,
											LPARAM dwInitParam,
											const DialogCreationData& CreationData);
	public:
		static UIManager*			Get();
		static bool					Initialize(const char* MainWindowClassName, HMENU MainMenuHandle = nullptr);
		static void					Deinitialize();

		int							MsgBoxI(HWND Parent, UINT Flags, const char* Format, ...);
		int							MsgBoxI(UINT Flags, const char* Format, ...);
		int							MsgBoxI(const char* Format, ...);

		int							MsgBoxW(HWND Parent, UINT Flags, const char* Format, ...);
		int							MsgBoxW(UINT Flags, const char* Format, ...);
		int							MsgBoxW(const char* Format, ...);

		int							MsgBoxE(HWND Parent, UINT Flags, const char* Format, ...);
		int							MsgBoxE(UINT Flags, const char* Format, ...);
		int							MsgBoxE(const char* Format, ...);

		HWND						ModelessDialog(HINSTANCE hInstance,
												LPSTR lpTemplateName,
												HWND hWndParent,
												DLGPROC lpDialogFunc,
												LPARAM dwInitParam = NULL,
												bool Override = false);
		INT_PTR						ModalDialog(HINSTANCE hInstance,
												LPSTR lpTemplateName,
												HWND hWndParent,
												DLGPROC lpDialogFunc,
												LPARAM dwInitParam = NULL,
												bool Override = false);

		HWND						GetMainWindow(void) const;
		WindowHandleCollection*		GetWindowHandleCollection(UInt8 ID);
		WindowSubclasser*			GetSubclasser(void);
		DialogTemplateHotSwapper*	GetDialogHotSwapper(void);
		MenuTemplateHotSwapper*		GetMenuHotSwapper(void);
		WindowStyler*				GetWindowStyler(void);
		WindowInvalidationManager*	GetInvalidationManager(void);
	};
#define BGSEEUI									bgsee::UIManager::Get()
}