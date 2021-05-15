#include "WindowSubclasser.h"
#include "Console.h"

namespace bgsee
{
	int	WindowExtraData::GIC = 0;

	WindowExtraData::WindowExtraData(Type TypeID) :
		TypeID(TypeID)
	{
		GIC++;
	}

	WindowExtraData::~WindowExtraData()
	{
		GIC--;
		SME_ASSERT(GIC >= 0);
	}

	const WindowExtraData::Type WindowExtraData::GetTypeID(void) const
	{
		return TypeID;
	}

	WindowExtraDataCollection::WindowExtraDataCollection() :
		DataStore()
	{
		;//
	}

	WindowExtraDataCollection::~WindowExtraDataCollection()
	{
		DataStore.clear();
	}

	bool WindowExtraDataCollection::Add( WindowExtraData* Data )
	{
		SME_ASSERT(Data);

		if (Lookup(Data->GetTypeID()))
			return false;
		else
			DataStore.insert(std::make_pair(Data->GetTypeID(), Data));

		return true;
	}

	bool WindowExtraDataCollection::Remove( WindowExtraData::Type ID )
	{
		if (Lookup(ID) == nullptr)
			return false;
		else
			DataStore.erase(ID);

		return true;
	}

	WindowExtraData* WindowExtraDataCollection::Lookup(WindowExtraData::Type ID)
	{
		for (ExtraDataMapT::iterator Itr = DataStore.begin(); Itr != DataStore.end(); Itr++)
		{
			if (Itr->first == ID)
				return Itr->second;
		}

		return nullptr;
	}

	bgsee::WindowSubclassProcCollection::SubclassDataArrayT::iterator WindowSubclassProcCollection::Find(SubclassProc Proc)
	{
		for (auto Itr = DataStore.begin(); Itr != DataStore.end(); ++Itr)
		{
			if (Itr->Subclass == Proc)
				return Itr;
		}

		return DataStore.end();
	}

	void WindowSubclassProcCollection::Resort()
	{
		std::stable_sort(DataStore.begin(), DataStore.end(), [](const auto& a, const auto& b) -> bool {
			return a.Priority > b.Priority;
		});
	}

	WindowSubclassProcCollection::WindowSubclassProcCollection()
	{
		DataStore.reserve(10);
		NextPriority = 0;
	}

	WindowSubclassProcCollection::~WindowSubclassProcCollection()
	{
		DataStore.clear();
	}

	bool WindowSubclassProcCollection::Add(SubclassProc SubclassProc, int Priority, const SubclassPredicateT& Predicate)
	{
		auto Match = Find(SubclassProc);
		if (Match != DataStore.end())
			return false;

		if (Priority == kPriority_Default)
			Priority = --NextPriority;

		DataStore.emplace_back(Priority, SubclassProc, Predicate);
		Resort();
		return true;
	}

	bool WindowSubclassProcCollection::Remove(SubclassProc SubclassProc)
	{
		auto Match = Find(SubclassProc);
		if (Match == DataStore.end())
			return false;

		DataStore.erase(Match);
		Resort();
		return true;
	}

	void WindowSubclassProcCollection::Clear()
	{
		DataStore.clear();
		NextPriority = 0;
	}

	void WindowSubclassProcCollection::Merge(const WindowSubclassProcCollection& Source)
	{
		std::unordered_set<SubclassProc> ProcSet;

		for (const auto& Itr : DataStore)
			ProcSet.emplace(Itr.Subclass);

		for (const auto& Itr : Source.DataStore)
		{
			if (ProcSet.find(Itr.Subclass) == ProcSet.end())
				DataStore.emplace_back(Itr.Priority, Itr.Subclass, Itr.Predicate);
		}

		Resort();
	}

	bgsee::WindowSubclassProcCollection::SubclassProcArrayT WindowSubclassProcCollection::GetSubclasses(HWND PredicateArgument) const
	{
		SubclassProcArrayT Out;

		for (const auto& Itr : DataStore)
		{
			if (Itr.Predicate == nullptr || Itr.Predicate(PredicateArgument))
				Out.emplace_back(Itr.Subclass);
		}

		return Out;
	}

	DialogCreationData::DialogCreationData() :
		Magic(kMagic)
	{
		Modal = false;
		SkipSubclassing = false;
		InstantiationTemplate = kInvalidResourceTemplateOrdinal;
		CreationDlgProc = nullptr;
		CreationUserData = 0;
	}

	DialogCreationData::DialogCreationData(const DialogCreationData& rhs) :
		Magic(kMagic)
	{
		*this = rhs;
	}

	const DialogCreationData& DialogCreationData::operator=(const DialogCreationData& rhs)
	{
		this->Modal = rhs.Modal;
		this->SkipSubclassing = rhs.SkipSubclassing;
		this->InstantiationTemplate = rhs.InstantiationTemplate;
		this->CreationDlgProc = rhs.CreationDlgProc;
		this->CreationUserData = rhs.CreationUserData;
		return *this;
	}

	bool DialogCreationData::TryGetFromLParam(LPARAM lParam, DialogCreationData& OutData)
	{
		if (lParam == 0)
			return false;

		// HACK!
		__try
		{
			auto PointerValue = *reinterpret_cast<UInt32*>(lParam);
			if (PointerValue == kMagic)
			{
				auto PassedData = reinterpret_cast<DialogCreationData*>(lParam);
				OutData = *PassedData;

				return true;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}

		return false;
	}

	WindowSubclasser::SubclassedWindowData::SubclassedWindowData()
	{
		Type = SubclassType::Window;
		OriginalProc.Window = nullptr;
		QueuedForDeletion = false;
		DisableSubclassProcessing = false;
	}

	LRESULT WindowSubclasser::SubclassedWindowData::ProcessSubclasses(WindowSubclasser* Subclasser, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return)
	{
		LRESULT Result = FALSE;

		if (QueuedForDeletion)
			return Result;
		else if (DisableSubclassProcessing)
			return Result;

		// work on a buffer as the subclass array can change inside a callback
		auto SubclassesCopy(Subclasses);
		for (const auto& Itr : SubclassesCopy)
		{
			// break early if the window was destroyed inside a previous callback or if a shutdown is taking place
			if (Subclasser->IsShuttingDown())
			{
				Return = true;
				return FALSE;
			}
			else if (QueuedForDeletion)
				break;

			WindowSubclassProcCollection::SubclassProcExtraParams xParams;
			xParams.In.PreviousSubclassHandledMessage = Return;
			xParams.In.ExtraData = &ExtraData;
			xParams.In.Subclasser = Subclasser;

			auto CurrentResult = Itr(hWnd, uMsg, wParam, lParam, &xParams);
			if (Return == false && xParams.Out.MarkMessageAsHandled)
			{
				Result = CurrentResult;
				Return = true;
			}
		}

		return Result;
	}

	LRESULT WindowSubclasser::SubclassedWindowData::SendMessageToOrgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool PreventSubclassProcessing)
	{
		if (PreventSubclassProcessing)
		{
			SME_ASSERT(DisableSubclassProcessing == false);
			SME::MiscGunk::ScopedSetter<bool> GuardTunnelling(DisableSubclassProcessing, true);

			if (Type == SubclassType::Window)
			{
				SME_ASSERT(OriginalProc.Window);
				return CallWindowProc(OriginalProc.Window, hWnd, uMsg, wParam, lParam);
			}
			else
			{
				SME_ASSERT(OriginalProc.Dialog);
				return CallWindowProc((WNDPROC)OriginalProc.Dialog, hWnd, uMsg, wParam, lParam);
			}
		}

		if (Type == SubclassType::Window)
		{
			SME_ASSERT(OriginalProc.Window);
			return CallWindowProc(OriginalProc.Window, hWnd, uMsg, wParam, lParam);
		}
		else
		{
			SME_ASSERT(OriginalProc.Dialog);
			return CallWindowProc((WNDPROC)OriginalProc.Dialog, hWnd, uMsg, wParam, lParam);
		}
	}

	const std::vector<std::string> WindowSubclasser::BlacklistedWindowClasses
	{
		"WindowsForms",
		"HwndWrapper"
	};

	bool WindowSubclasser::IsWindowBlacklisted(HWND hWnd, UINT uMsg) const
	{
		char WindowClassName[256] = {};
		GetClassName(hWnd, WindowClassName, ARRAYSIZE(WindowClassName));

		// custom dialog boxes are sent the WM_CREATE message before the WM_INITDIALOG message
		// those messages should not be handled
		if (strcmp(WindowClassName, "#32770") == 0 && uMsg == WM_CREATE)
			return true;

		// skip all non-native windows/dialogs and their children, amongst others that we don't want to hook
		HWND NextWindow = hWnd;
		while (NextWindow != NULL)
		{
			WindowClassName[0] = '\0';
			GetClassName(NextWindow, WindowClassName, ARRAYSIZE(WindowClassName));
			std::string Wrapper(WindowClassName);

			for (const auto& Itr : BlacklistedWindowClasses)
			{
				if (Wrapper.find(Itr) == 0)
					return true;
			}

			auto NextAncestor = GetAncestor(NextWindow, GA_PARENT);
			if (NextAncestor == NextWindow)
				break;

			NextWindow = NextAncestor;
		}

		return false;
	}

	LRESULT WindowSubclasser::WindowsHookCallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HC_ACTION)
		{
			auto MessageData = reinterpret_cast<CWPSTRUCT*>(lParam);
			if (IsWindowBlacklisted(MessageData->hwnd, MessageData->message))
				return CallNextHookEx(nullptr, nCode, wParam, lParam);

			switch (MessageData->message)
			{
			case WM_CREATE:
				if (ShouldSubclassWindowOnCreation(MessageData->hwnd))
					ApplyWindowSubclass(MessageData->hwnd);

				break;
			case WM_INITDIALOG:
			{
				DialogCreationData CreationData;
				if (DialogCreationData::TryGetFromLParam(MessageData->lParam, CreationData) == false)
				{
					// replace with the params at hand to facilitate tracking
					CreationData.Modal = IsWindowEnabled(GetParent(MessageData->hwnd)) == FALSE;
					CreationData.CreationUserData = MessageData->lParam;
				}

				// the WM_INITDIALOG message can be sent to an existing dialog to re-initialize it
				// permanent dialogs like the object, cell and render windows tend to do this
				if (ActiveThreadDialogHandles.find(MessageData->hwnd) == ActiveThreadDialogHandles.end())
					ActiveThreadDialogHandles.emplace(MessageData->hwnd, new DialogCreationData(CreationData));


				// we need to subclass all dialog boxes with valid creation data unconditionally
				// so that the WM_INITDIALOG message is correctly handled by the original subclass
				// i.e, since we can't modify the message params from inside the hook to fix-up the original userdata, etc
				if (CreationData.IsValid())
					ApplyDialogSubclass(MessageData->hwnd, CreationData);

				break;
			}
			case WM_DESTROY:
				ActiveThreadDialogHandles.erase(MessageData->hwnd);

				break;
			}
		}

		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	}


	LRESULT WindowSubclasser::BaseSubclassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		ProcessStackOperator StackOperator(*this, hWnd);

		bool CallbackReturn = false;
		LRESULT ProcResult = FALSE;
		auto SubclassedData = ActiveSubclasses.at(hWnd).get();
		SME_ASSERT(SubclassedData->QueuedForDeletion == false);

		switch (uMsg)
		{
		case WM_CREATE:
		case WM_INITDIALOG:
		{
			auto IsDialog = SubclassedData->IsDialog();
			auto PreMessageId = IsDialog ? WM_SUBCLASSER_PRE_WM_INITDIALOG : WM_SUBCLASSER_PRE_WM_CREATE;
			auto PostMessageId = IsDialog ? WM_SUBCLASSER_POST_WM_INITDIALOG : WM_SUBCLASSER_POST_WM_CREATE;
			auto FirstLParam = lParam;
			bool Throwaway = false;
			if (IsDialog)
				FirstLParam = SubclassedData->DialogSpecific.CreationData.CreationUserData;

			SubclassedData->ProcessSubclasses(this, hWnd, PreMessageId, wParam, FirstLParam, Throwaway);

			// window must not be destroyed inside the pre-init callback
			SME_ASSERT(SubclassedData->QueuedForDeletion == false);

			// execute the original proc's handler
			ProcResult = SubclassedData->SendMessageToOrgProc(hWnd, uMsg, wParam, FirstLParam, false);

			bool SkipSubclassing = IsDialog && (SubclassedData->DialogSpecific.CreationData.IsValid() == false ||
									SubclassedData->DialogSpecific.CreationData.SkipSubclassing ||
									ShouldSubclassDialogOnCreation(hWnd, SubclassedData->DialogSpecific.CreationData) == false);
			if (SkipSubclassing)
			{
				// remove subclass immediately if the dialog originally wanted to skip subclassing
				// or if it has no queued subclass procs
				RemoveSubclass(hWnd);
				return ProcResult;
			}

			// window could have been destroyed by the original window proc
			if (SubclassedData->QueuedForDeletion == false)
			{
				ProcResult = SubclassedData->ProcessSubclasses(this, hWnd, uMsg, wParam, FirstLParam, CallbackReturn);

				if (SubclassedData->QueuedForDeletion == false)
					SubclassedData->ProcessSubclasses(this, hWnd, PostMessageId, wParam, FirstLParam, Throwaway);
			}


			return ProcResult;
		}
		case WM_SUBCLASSER_INTERNAL_REMOVESUBCLASS:
		case WM_DESTROY:
		{
			bool IsShuttingDown = uMsg == WM_SUBCLASSER_INTERNAL_REMOVESUBCLASS && wParam == TRUE;
			bool DoNotNotifyUserSubclasses = uMsg == WM_SUBCLASSER_INTERNAL_REMOVESUBCLASS && lParam == FALSE;

			if (DoNotNotifyUserSubclasses == false)
				SubclassedData->ProcessSubclasses(this, hWnd, WM_DESTROY, wParam, lParam, CallbackReturn);

			// no need to invoke the original handler in case of an internal release event
			// as we only care about our own code that might potentially handle it
			if (uMsg != WM_SUBCLASSER_INTERNAL_REMOVESUBCLASS)
				ProcResult = SubclassedData->SendMessageToOrgProc(hWnd, uMsg, wParam, lParam, false);

			RemoveSubclass(hWnd);
			return ProcResult;
		}
		}

		// all other messages
		ProcResult = SubclassedData->ProcessSubclasses(this, hWnd, uMsg, wParam, lParam, CallbackReturn);
		if (CallbackReturn)
			return ProcResult;

		// message was not handled by any of the subclasses, forward to the original
		if (SubclassedData->QueuedForDeletion == false)
			return SubclassedData->SendMessageToOrgProc(hWnd, uMsg, wParam, lParam, false);

		return FALSE;
	}

	void WindowSubclasser::DeletionTimerProc(HWND hWnd, UINT uMsg, UINT_PTR uID, DWORD dwTickCount)
	{
		// just clear the deletion queue periodically
		DeletionQueue.clear();
	}

	bool WindowSubclasser::ShouldSubclassWindowOnCreation(HWND hWnd) const
	{
		if (GlobalSubclassProcs.Size() != 0)
			return true;

		return false;
	}

	bool WindowSubclasser::ShouldSubclassDialogOnCreation(HWND hWnd, const DialogCreationData& CreationData) const
	{
		if (CreationData.SkipSubclassing)
			return false;

		if (GlobalSubclassProcs.Size() != 0)
			return true;

		if (ResourceTemplateSpecificSubclassProcs.find(CreationData.InstantiationTemplate) != ResourceTemplateSpecificSubclassProcs.end())
			return true;

		return false;
	}

	void WindowSubclasser::ApplyWindowSubclass(HWND hWnd)
	{
		SME_ASSERT(IsWindowBlacklisted(hWnd, WM_NULL) == false);
		SME_ASSERT(ActiveSubclasses.find(hWnd) == ActiveSubclasses.end());

		std::unique_ptr<SubclassedWindowData> NewSubclassData(new SubclassedWindowData);
		NewSubclassData->Type = SubclassedWindowData::SubclassType::Window;
		NewSubclassData->OriginalProc.Window = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWL_WNDPROC,
																reinterpret_cast<LONG_PTR>(ThunkBaseSubclassWndProc())));
		char WindowClassName[256] = {};
		GetClassName(hWnd, WindowClassName, ARRAYSIZE(WindowClassName));
		NewSubclassData->WindowClassName = WindowClassName;

		RebuildSubclassProcs(hWnd, *NewSubclassData.get());
		ActiveSubclasses.emplace(hWnd, std::move(NewSubclassData));
	}

	void WindowSubclasser::ApplyDialogSubclass(HWND hWnd, const DialogCreationData& CreationData)
	{
		SME_ASSERT(IsWindowBlacklisted(hWnd, WM_NULL) == false);
		SME_ASSERT(ActiveSubclasses.find(hWnd) == ActiveSubclasses.end());

		std::unique_ptr<SubclassedWindowData> NewSubclassData(new SubclassedWindowData);
		NewSubclassData->Type = SubclassedWindowData::SubclassType::Dialog;
		NewSubclassData->OriginalProc.Dialog = reinterpret_cast<DLGPROC>(SetWindowLongPtr(hWnd, DWL_DLGPROC,
																reinterpret_cast<LONG_PTR>(ThunkBaseSubclassWndProc())));
		char WindowClassName[256] = {};
		GetClassName(hWnd, WindowClassName, ARRAYSIZE(WindowClassName));
		NewSubclassData->WindowClassName = WindowClassName;
		NewSubclassData->DialogSpecific.CreationData = CreationData;

		RebuildSubclassProcs(hWnd, *NewSubclassData.get());
		ActiveSubclasses.emplace(hWnd, std::move(NewSubclassData));
	}

	void WindowSubclasser::RebuildSubclassProcs(HWND hWnd, SubclassedWindowData& SubclassData) const
	{
		WindowSubclassProcCollection ProcCollection;

		ProcCollection.Merge(GlobalSubclassProcs);

		if (SubclassData.IsDialog())
		{
			auto ResourceTemplate = SubclassData.DialogSpecific.CreationData.InstantiationTemplate;
			auto Match = ResourceTemplateSpecificSubclassProcs.find(ResourceTemplate);
			if (Match != ResourceTemplateSpecificSubclassProcs.end())
				ProcCollection.Merge(*Match->second);
		}


		auto Match = HandleSpecificSubclassProcs.find(hWnd);
		if (Match != HandleSpecificSubclassProcs.end())
			ProcCollection.Merge(*Match->second);

		auto SortedProcs(ProcCollection.GetSubclasses(hWnd));
		SubclassData.Subclasses.swap(SortedProcs);
	}

	void WindowSubclasser::RebuildAllSubclassProcs() const
	{
		for (auto& Itr : ActiveSubclasses)
			RebuildSubclassProcs(Itr.first, *Itr.second.get());
	}

	void WindowSubclasser::RemoveSubclass(HWND hWnd)
	{
		auto Match = ActiveSubclasses.find(hWnd);
		SME_ASSERT(Match != ActiveSubclasses.end());

		auto& SubclassData = Match->second;
		SME_ASSERT(SubclassData->QueuedForDeletion == false);

		SubclassData->QueuedForDeletion = true;

		if (SubclassData->IsDialog())
			SetWindowLongPtr(hWnd, DWL_DLGPROC, reinterpret_cast<LONG_PTR>(SubclassData->OriginalProc.Dialog));
		else
			SetWindowLongPtr(hWnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(SubclassData->OriginalProc.Window));

		DeletionQueue.emplace_back(SubclassData.release());

		ActiveSubclasses.erase(hWnd);

		// clean up handle-specific subclasses once the parent window is destroyed
		HandleSpecificSubclassProcs.erase(hWnd);
	}

	void WindowSubclasser::ToggleWindowsHook(bool Enabled)
	{
		SME_ASSERT(WindowsHookActive != Enabled);

		if (Enabled)
		{
			WindowsHook = SetWindowsHookEx(WH_CALLWNDPROC, ThunkWindowsHookCallWndProc(), nullptr, OwnerThreadId);
			SME_ASSERT(WindowsHook);
		}
		else
		{
			auto Result = UnhookWindowsHookEx(WindowsHook);
			WindowsHook = nullptr;
			SME_ASSERT(Result);
		}

		WindowsHookActive = WindowsHookActive == false;
	}

	WindowSubclasser::WindowSubclasser() :
		OwnerThreadId(GetCurrentThreadId()),
		ThunkWindowsHookCallWndProc(this, &WindowSubclasser::WindowsHookCallWndProc),
		ThunkBaseSubclassWndProc(this, &WindowSubclasser::BaseSubclassWndProc),
		ThunkDeletionTimerProc(this, &WindowSubclasser::DeletionTimerProc),
		WindowsHookActive(false),
		TearingDown(false)
	{
		DeletionTimer = SetTimer(nullptr, kDeletionTimerID, 1000, ThunkDeletionTimerProc());
		SME_ASSERT(DeletionTimer);

		ToggleWindowsHook(true);
	}

	WindowSubclasser::~WindowSubclasser()
	{
		ToggleWindowsHook(false);
		KillTimer(nullptr, DeletionTimer);

		// use a buffer to prevent iterator invalidation
		std::vector<HWND> ActiveSubclassesCopy;
		for (const auto& Itr : ActiveSubclasses)
			ActiveSubclassesCopy.emplace_back(Itr.first);

		for (const auto& Itr : ActiveSubclassesCopy)
			SendMessage(Itr, WM_SUBCLASSER_INTERNAL_REMOVESUBCLASS, TRUE, TRUE);

		TearingDown = true;

		DeletionQueue.clear();
		ActiveThreadDialogHandles.clear();

		if (WindowExtraData::GIC)
		{
			BGSEECONSOLE_MESSAGE("WindowSubclasser::D'tor - Session leaked %d instances of WindowExtraData!", WindowExtraData::GIC);
			SHOW_LEAKAGE_MESSAGE("WindowSubclasser");
		}
	}

	void WindowSubclasser::RegisterGlobalSubclass(WindowSubclassProcCollection::SubclassProc SubclassProc, int Priority,
												const WindowSubclassProcCollection::SubclassPredicateT& Predicate)
	{
		if (GlobalSubclassProcs.Add(SubclassProc, Priority, Predicate))
			RebuildAllSubclassProcs();
	}

	void WindowSubclasser::DeregisterGlobalSubclass(WindowSubclassProcCollection::SubclassProc SubclassProc)
	{
		if (GlobalSubclassProcs.Remove(SubclassProc))
			RebuildAllSubclassProcs();
	}

	void WindowSubclasser::RegisterSubclassForWindow(HWND hWnd, WindowSubclassProcCollection::SubclassProc SubclassProc, int Priority)
	{
		auto Match = HandleSpecificSubclassProcs.find(hWnd);
		if (Match == HandleSpecificSubclassProcs.end())
			Match = HandleSpecificSubclassProcs.emplace(hWnd, new WindowSubclassProcCollection).first;

		if (Match->second->Add(SubclassProc, Priority) == false)
		{
			if (Match->second->Size() == 0)
				HandleSpecificSubclassProcs.erase(Match);
			return;
		}

		auto ActiveSubclass = ActiveSubclasses.find(hWnd);
		if (ActiveSubclass != ActiveSubclasses.end())
		{
			RebuildSubclassProcs(hWnd, *ActiveSubclass->second);
			return;
		}

		auto ActiveDialog = ActiveThreadDialogHandles.find(hWnd);
		if (ActiveDialog != ActiveThreadDialogHandles.end())
			ApplyDialogSubclass(hWnd, *ActiveDialog->second);
		else
			ApplyWindowSubclass(hWnd);
	}

	void WindowSubclasser::DeregisterSubclassForWindow(HWND hWnd, WindowSubclassProcCollection::SubclassProc SubclassProc)
	{
		auto Match = HandleSpecificSubclassProcs.find(hWnd);
		if (Match == HandleSpecificSubclassProcs.end())
			return;

		if (Match->second->Remove(SubclassProc) == false)
			return;

		if (Match->second->Size() == 0)
		{
			SendMessage(Match->first, WM_SUBCLASSER_INTERNAL_REMOVESUBCLASS, FALSE, TRUE);
			return;
		}

		auto ActiveSubclass = ActiveSubclasses.find(hWnd);
		if (ActiveSubclass != ActiveSubclasses.end())
			RebuildSubclassProcs(hWnd, *ActiveSubclass->second);
	}

	void WindowSubclasser::RegisterSubclassForDialogResourceTemplate(ResourceTemplateOrdinalT Ordinal,
																	WindowSubclassProcCollection::SubclassProc SubclassProc, int Priority,
																	const WindowSubclassProcCollection::SubclassPredicateT& Predicate)
	{
		auto Match = ResourceTemplateSpecificSubclassProcs.find(Ordinal);
		if (Match == ResourceTemplateSpecificSubclassProcs.end())
			Match = ResourceTemplateSpecificSubclassProcs.emplace(Ordinal, new WindowSubclassProcCollection).first;

		if (Match->second->Add(SubclassProc, Priority, Predicate) == false)
		{
			if (Match->second->Size() == 0)
			{
				ResourceTemplateSpecificSubclassProcs.erase(Match);
				for (const auto& Itr : ActiveSubclasses)
					SendMessage(Itr.first, WM_SUBCLASSER_INTERNAL_REMOVESUBCLASS, FALSE, TRUE);
			}

			return;
		}

		for (const auto& Itr : ActiveSubclasses)
		{
			if (Itr.second->IsDialog() && Itr.second->DialogSpecific.CreationData.InstantiationTemplate == Ordinal)
				RebuildSubclassProcs(Itr.first, *Itr.second);
		}
	}

	void WindowSubclasser::DeregisterSubclassForDialogResourceTemplate(ResourceTemplateOrdinalT Ordinal,
																	WindowSubclassProcCollection::SubclassProc SubclassProc)
	{
		auto Match = ResourceTemplateSpecificSubclassProcs.find(Ordinal);
		if (Match == ResourceTemplateSpecificSubclassProcs.end())
			return;

		if (Match->second->Remove(SubclassProc) == false)
			return;

		if (Match->second->Size() == 0)
		{
			ResourceTemplateSpecificSubclassProcs.erase(Match);
			for (const auto& Itr : ActiveSubclasses)
				SendMessage(Itr.first, WM_SUBCLASSER_INTERNAL_REMOVESUBCLASS, FALSE, TRUE);

			return;
		}

		for (const auto& Itr : ActiveSubclasses)
		{
			if (Itr.second->IsDialog() && Itr.second->DialogSpecific.CreationData.InstantiationTemplate == Ordinal)
				RebuildSubclassProcs(Itr.first, *Itr.second);
		}
	}

	LRESULT WindowSubclasser::TunnelMessageToOrgWndProc(HWND SubclassedWindow, UINT uMsg, WPARAM wParam, LPARAM lParam, bool SuppressSubclasses) const
	{
		auto ActiveSubclass = ActiveSubclasses.find(SubclassedWindow);
		SME_ASSERT(ActiveSubclass != ActiveSubclasses.end());

		return ActiveSubclass->second->SendMessageToOrgProc(ActiveSubclass->first, uMsg, wParam, lParam, SuppressSubclasses);
	}

	ResourceTemplateOrdinalT WindowSubclasser::GetDialogTemplate(HWND SubclassedDialog) const
	{
		auto ActiveSubclass = ActiveSubclasses.find(SubclassedDialog);
		SME_ASSERT(ActiveSubclass != ActiveSubclasses.end());

		return ActiveSubclass->second->DialogSpecific.CreationData.InstantiationTemplate;
	}


	void WindowSubclasser::SuspendHooks()
	{
		ToggleWindowsHook(false);
	}

	void WindowSubclasser::ResumeHooks()
	{
		ToggleWindowsHook(true);

	}

	bool WindowSubclasser::IsShuttingDown() const
	{
		return TearingDown;
	}

	bool WindowSubclasser::IsWindowSubclassed(HWND hWnd) const
	{
		return ActiveSubclasses.find(hWnd) != ActiveSubclasses.end();
	}

	bool WindowSubclasser::IsWindowADialog(HWND SubclassedWindow) const
	{
		auto ActiveSubclass = ActiveSubclasses.find(SubclassedWindow);
		SME_ASSERT(ActiveSubclass != ActiveSubclasses.end());

		return ActiveSubclass->second->IsDialog();
	}

	HWND WindowSubclasser::GetMostRecentWindowHandle() const
	{
		if (ProcessingHandles.empty())
			return nullptr;

		return ProcessingHandles.top();
	}

}

