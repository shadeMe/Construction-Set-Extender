#pragma once

#include "UICommon.h"
#include "HookUtil.h"

namespace bgsee
{
	class WindowSubclasser;

	class WindowExtraData
	{
	public:
		typedef UInt32 Type;
	protected:
		friend class WindowSubclasser;
		static int			GIC;

		Type	TypeID;
	public:
		WindowExtraData(Type TypeID);
		virtual ~WindowExtraData() = 0;

		virtual const Type GetTypeID(void) const;
	};

	class WindowExtraDataCollection
	{
		typedef std::unordered_map<WindowExtraData::Type, WindowExtraData*>
							ExtraDataMapT;

		ExtraDataMapT		DataStore;
	public:
		WindowExtraDataCollection();
		~WindowExtraDataCollection();

		bool				Add(WindowExtraData* Data);		// caller retains the ownership of the pointer
		bool				Remove(WindowExtraData::Type ID);
		WindowExtraData*	Lookup(WindowExtraData::Type ID);
	};

// For quick 'n dirty lookup
#define BGSEE_GETWINDOWXDATA(xDataClass, xDataCollection)	reinterpret_cast<xDataClass*>(xDataCollection->Lookup(xDataClass::kTypeID))

	class WindowSubclassProcCollection
	{
	public:
		struct SubclassProcExtraParams
		{
			struct Inputs
			{
				bool PreviousSubclassHandledMessage = false;		// set to true if a previous subclass call explicitly handled the message and returned a value
				WindowExtraDataCollection* ExtraData = nullptr;
				WindowSubclasser* Subclasser = nullptr;
			} In;

			struct Outputs
			{
				bool MarkMessageAsHandled = false;					// must be set to true on return to explicitly mark the message as handled
																	// the return value of the first subclass proc call to mark this will be final return value for the message
			} Out;
		};

		typedef LRESULT (CALLBACK* SubclassProc)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SubclassProcExtraParams* SubclassParams);

		typedef std::vector<SubclassProc>
							SubclassProcArrayT;
		typedef std::function<bool(HWND)>
							SubclassPredicateT;

		enum { kPriority_Default = 0 };
	private:
		struct Data
		{
			int					Priority;
			SubclassProc		Subclass;
			SubclassPredicateT	Predicate;

			Data(int Priority, SubclassProc Subclass, const SubclassPredicateT& Predicate)
				: Priority(Priority), Subclass(Subclass), Predicate(Predicate) {}
		};

		typedef std::vector<Data>
							SubclassDataArrayT;

		SubclassDataArrayT	DataStore;
		int					NextPriority;

		SubclassDataArrayT::iterator
							Find(SubclassProc Proc);
		void				Resort();
	public:
		WindowSubclassProcCollection();
		~WindowSubclassProcCollection();

		bool				Add(SubclassProc SubclassProc, int Priority = kPriority_Default, const SubclassPredicateT& Predicate = nullptr);
		bool				Remove(SubclassProc SubclassProc);
		void				Clear();
		void				Merge(const WindowSubclassProcCollection& Source);
		UInt32				Size() const { return DataStore.size(); }
		SubclassProcArrayT	GetSubclasses(HWND PredicateArgument = NULL) const;
	};

	template <typename Class>
	using SubclassProcThunk = util::ThunkStdCall<Class, LRESULT, HWND, UINT, WPARAM, LPARAM, bgsee::WindowSubclassProcCollection::SubclassProcExtraParams*>;


	// Used by the UIManager to communicate extra information
	// about a newly created dialog to the subclasser
	struct DialogCreationData
	{
		static constexpr UInt32	kMagic = 'BSEE';

		const UInt32				Magic;
		bool						SkipSubclassing;
		bool						Modal;
		ResourceTemplateOrdinalT	InstantiationTemplate;
		DLGPROC						CreationDlgProc;
		LPARAM						CreationUserData;

		DialogCreationData();
		DialogCreationData(const DialogCreationData& rhs);
		const DialogCreationData& operator=(const DialogCreationData& rhs);

		bool		IsValid() const { return InstantiationTemplate != kInvalidResourceTemplateOrdinal; }
		static bool TryGetFromLParam(LPARAM lParam, DialogCreationData& OutData);
	};


	class WindowSubclasser
	{
		// Assigned to every HWND that gets subclassed
		struct SubclassedWindowData
		{
			enum class SubclassType
			{
				Window,
				Dialog,
			};

			SubclassType	Type;
			std::string		WindowClassName;
			union
			{
				WNDPROC		Window;
				DLGPROC		Dialog;
			} OriginalProc;
			struct
			{
				DialogCreationData	CreationData;
			} DialogSpecific;

			WindowExtraDataCollection
							ExtraData;
			WindowSubclassProcCollection::SubclassProcArrayT
							Subclasses;
			bool			QueuedForDeletion;
			bool			DisableSubclassProcessing;

			SubclassedWindowData();

			bool		IsDialog() const { return Type == SubclassType::Dialog; }
			LRESULT		ProcessSubclasses(WindowSubclasser* Subclasser, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return);
			LRESULT		SendMessageToOrgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool PreventSubclassProcessing);
		};

		static const std::vector<const char*> BlacklistedWindowClasses;
		static const std::vector<const char*> BlacklistedDialogWindowClasses;

		typedef std::unordered_map<HWND, std::unique_ptr<SubclassedWindowData>> HandleToSubclassDataMapT;
		typedef std::vector<std::unique_ptr<SubclassedWindowData>> SubclassDataArrayT;
		typedef std::unordered_map<ResourceTemplateOrdinalT, std::unique_ptr<WindowSubclassProcCollection>> OrdinalToSubclassesMapT;
		typedef std::unordered_map<HWND, std::unique_ptr<WindowSubclassProcCollection>> HandleToSubclassesMapT;
		typedef std::unordered_map<HWND, std::unique_ptr<DialogCreationData>> HandleToDialogCreationDataMapT;
		typedef std::stack<HWND> HandleStackT;

		struct ProcessStackOperator
		{
			WindowSubclasser&	Parent;

			ProcessStackOperator(WindowSubclasser& Parent, HWND hWnd) : Parent(Parent)
				{ Parent.ProcessingHandles.push(hWnd); }
			~ProcessStackOperator()
				{ Parent.ProcessingHandles.pop(); }
		};

		static constexpr UINT_PTR	kDeletionTimerID = 'WSDT';

		DWORD		OwnerThreadId;
		HandleToDialogCreationDataMapT
					ActiveThreadDialogHandles;
		HandleToSubclassDataMapT
					ActiveSubclasses;
		WindowSubclassProcCollection
					GlobalSubclassProcs;
		OrdinalToSubclassesMapT
					ResourceTemplateSpecificSubclassProcs;
		HandleToSubclassesMapT
					HandleSpecificSubclassProcs;
		SubclassDataArrayT
					DeletionQueue;
		HandleStackT
					ProcessingHandles;
		UINT_PTR	DeletionTimer;
		HHOOK		WindowsHook;
		bool		WindowsHookActive;
		bool		TearingDown;

		util::ThunkStdCall<WindowSubclasser, LRESULT, int, WPARAM, LPARAM>
					ThunkWindowsHookCallWndProc;
		util::ThunkStdCall<WindowSubclasser, LRESULT, HWND, UINT, WPARAM, LPARAM>
					ThunkBaseSubclassWndProc;
		util::ThunkStdCall<WindowSubclasser, void, HWND, UINT, UINT_PTR, DWORD>
					ThunkDeletionTimerProc;

		bool		IsWindowBlacklisted(HWND hWnd, UINT uMsg) const;

		LRESULT 	WindowsHookCallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
		LRESULT		BaseSubclassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void		DeletionTimerProc(HWND hWnd, UINT uMsg, UINT_PTR uID, DWORD dwTickCount);

		bool		ShouldSubclassWindowOnCreation(HWND hWnd) const;
		bool		ShouldSubclassDialogOnCreation(HWND hWnd, const DialogCreationData& CreationData) const;

		void		ApplyWindowSubclass(HWND hWnd);
		void		ApplyDialogSubclass(HWND hWnd, const DialogCreationData& CreationData);

		void		RebuildSubclassProcs(HWND hWnd, SubclassedWindowData& SubclassData) const;
		void		RebuildAllSubclassProcs() const;
		void		RemoveSubclass(HWND hWnd);

		void		ToggleWindowsHook(bool Enabled);
	public:
		WindowSubclasser();
		~WindowSubclasser();

		void	RegisterGlobalSubclass(WindowSubclassProcCollection::SubclassProc SubclassProc,
									int Priority = WindowSubclassProcCollection::kPriority_Default,
									const WindowSubclassProcCollection::SubclassPredicateT& Predicate = nullptr);
		void	DeregisterGlobalSubclass(WindowSubclassProcCollection::SubclassProc SubclassProc);

		void	RegisterSubclassForWindow(HWND hWnd, WindowSubclassProcCollection::SubclassProc SubclassProc,
										int Priority = WindowSubclassProcCollection::kPriority_Default);
		void	DeregisterSubclassForWindow(HWND hWnd, WindowSubclassProcCollection::SubclassProc SubclassProc);

		void	RegisterSubclassForDialogResourceTemplate(ResourceTemplateOrdinalT Ordinal,
														WindowSubclassProcCollection::SubclassProc SubclassProc,
														int Priority = WindowSubclassProcCollection::kPriority_Default,
														const WindowSubclassProcCollection::SubclassPredicateT& Predicate = nullptr);
		void	DeregisterSubclassForDialogResourceTemplate(ResourceTemplateOrdinalT Ordinal,
			WindowSubclassProcCollection::SubclassProc SubclassProc);

		LRESULT	TunnelMessageToOrgWndProc(HWND SubclassedWindow, UINT uMsg, WPARAM wParam, LPARAM lParam, bool SuppressSubclasses) const;
		ResourceTemplateOrdinalT GetDialogTemplate(HWND SubclassedDialog) const;

		void	SuspendHooks();
		void	ResumeHooks();

		bool	IsShuttingDown() const;
		bool	IsWindowSubclassed(HWND hWnd) const;
		bool	IsWindowADialog(HWND SubclassedWindow) const;
		HWND	GetMostRecentWindowHandle() const;
	};

#define WM_SUBCLASSER_INTERNAL_BEGIN			((WM_USER) + 0x100)

// wParam = TRUE if shutting down, lPARAM = FALSE if user subclasses should not receive the WM_DESTROY message
#define WM_SUBCLASSER_INTERNAL_REMOVESUBCLASS	((WM_SUBCLASSER_INTERNAL_BEGIN) + 1)

#define WM_SUBCLASSER_PRE_WM_CREATE				((WM_SUBCLASSER_INTERNAL_BEGIN) + 2)
#define WM_SUBCLASSER_POST_WM_CREATE			((WM_SUBCLASSER_INTERNAL_BEGIN) + 3)
#define WM_SUBCLASSER_PRE_WM_INITDIALOG			((WM_SUBCLASSER_INTERNAL_BEGIN) + 4)
#define WM_SUBCLASSER_POST_WM_INITDIALOG		((WM_SUBCLASSER_INTERNAL_BEGIN) + 5)

}