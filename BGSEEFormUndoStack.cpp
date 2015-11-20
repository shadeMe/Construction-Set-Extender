#include "BGSEEFormUndoStack.h"
#include "BGSEEConsole.h"

namespace BGSEditorExtender
{
	BGSEEFormUndoStack*		BGSEEFormUndoStack::Singleton = NULL;
	const char*				BGSEEFormUndoStack::kMessageLogContextName = "Undo Stack";
	int						BGSEEFormUndoProxy::GIC = 0;

	BGSEEFormUndoProxy::BGSEEFormUndoProxy() :
		EditorID("<no-editorID>"),
		Modified(false)
	{
		GIC++;
	}

	const char* BGSEEFormUndoProxy::GetEditorID( void ) const
	{
		return EditorID.c_str();
	}

	BGSEEFormUndoProxy::~BGSEEFormUndoProxy()
	{
		GIC--;

		SME_ASSERT(GIC >= 0);
	}


	BGSEEFormUndoStack::BGSEEFormUndoStack() :
		Operator(NULL),
		UndoStack(),
		RedoStack(),
		ConsoleMessageContext(NULL),
		WalkingStacks(false),
		Initialized(false)
	{
		;//
	}

	void BGSEEFormUndoStack::ResetStack( UndoProxyStackT& Stack )
	{
		while (Stack.size())
			Stack.pop();
	}

	BGSEEFormUndoStack::~BGSEEFormUndoStack()
	{
		Reset();

		SAFEDELETE(Operator);

		if (ConsoleMessageContext)
			BGSEECONSOLE->UnregisterMessageLogContext(ConsoleMessageContext);

		Initialized = false;

		if (BGSEEFormUndoProxy::GIC)
		{
			BGSEECONSOLE_MESSAGE("BGSEEFormUndoStack::D'tor - Session leaked %d instances of BGSEEFormUndoProxy!", BGSEEFormUndoProxy::GIC);
			SHOW_LEAKAGE_MESSAGE("BGSEEFormUndoStack");
		}

		Singleton = NULL;
	}

	BGSEEFormUndoStack* BGSEEFormUndoStack::GetSingleton( void )
	{
		if (Singleton == NULL)
			Singleton = new BGSEEFormUndoStack();

		return Singleton;
	}

	bool BGSEEFormUndoStack::Initialize( BGSEEFormUndoStackOperator* Operator )
	{
		if (Initialized)
			return false;

		Initialized = true;

		SME_ASSERT(Operator);

		this->Operator = Operator;
		ConsoleMessageContext = BGSEECONSOLE->RegisterMessageLogContext(kMessageLogContextName);

		if (ConsoleMessageContext == NULL)
		{
			Initialized = false;

			BGSEECONSOLE_MESSAGE("Couldn't register console message log context");
		}

		return Initialized;
	}

	bool BGSEEFormUndoStack::Record( BGSEEFormUndoProxy* Proxy )
	{
		bool Result = false;

		SME_ASSERT(Initialized && Proxy && Operator && WalkingStacks == false);

		// reset any pending proxies on the redo stack
		ResetStack(RedoStack);

		if (Operator->GetIsFormTypeUndoable(Proxy->GetType()) == false)
		{
			BGSEECONSOLE->PrintToMessageLogContext(ConsoleMessageContext, "Couldn't record proxy - Invalid type %s", Proxy->GetTypeString());
		}
		else
		{
			UndoProxyHandleT Handle(Proxy);
			UndoStack.push(Handle);

			BGSEECONSOLE->PrintToMessageLogContext(ConsoleMessageContext, "Proxy %s (%08X) recorded. Stack size = %d",
												Proxy->GetEditorID(), Proxy->GetFormID(), UndoStack.size());

			Result = true;
		}

		if (Result == false)
		{
			// release proxy
			delete Proxy;
		}

		return Result;
	}

	void BGSEEFormUndoStack::WalkUndoStack( UInt8 Operation, UndoProxyStackT& Stack, UndoProxyStackT& AlternateStack )
	{
		SME_ASSERT(Operator && WalkingStacks == false);
		SME::MiscGunk::ScopedSetter<bool> GuardStackWalker(WalkingStacks, true);

		if (Stack.size())
		{
			UndoProxyHandleT Proxy = Stack.top();
			Stack.pop();

			BGSEEFormUndoProxy* AltProxy = NULL;
			if (Operator->PreUndoRedoCallback(Proxy.get(), &AltProxy))
			{
				SME_ASSERT(AltProxy);

				Proxy->Undo();
				Operator->PostUndoRedoCallback();

				UndoProxyHandleT AltHandle(AltProxy);		// this is basically a copy of the form data from before the undo/redo op
				AlternateStack.push(AltHandle);

				switch (Operation)
				{
				case kOperation_Undo:
					BGSEECONSOLE->PrintToMessageLogContext(ConsoleMessageContext, "Proxy %s (%08X) undone. Size [U=%d, R=%d]",
														Proxy->GetEditorID(), Proxy->GetFormID(), Stack.size(), AlternateStack.size());

					break;
				case kOperation_Redo:
					BGSEECONSOLE->PrintToMessageLogContext(ConsoleMessageContext, "Proxy %s (%08X) redone. Size [U=%d, R=%d]",
														Proxy->GetEditorID(), Proxy->GetFormID(), AlternateStack.size(), Stack.size());

					break;
				}
			}
			else
			{
				BGSEECONSOLE->PrintToMessageLogContext(ConsoleMessageContext, "PreUndoRedoCallback returned false - Proxy %s (%08X) discarded",
													Proxy->GetEditorID(), Proxy->GetFormID());
			}
		}
	}

	void BGSEEFormUndoStack::PerformUndo( void )
	{
		SME_ASSERT(Initialized);

		WalkUndoStack(kOperation_Undo, UndoStack, RedoStack);
	}

	void BGSEEFormUndoStack::PerformRedo( void )
	{
		SME_ASSERT(Initialized);

		WalkUndoStack(kOperation_Redo, RedoStack, UndoStack);
	}

	void BGSEEFormUndoStack::Reset( void )
	{
		SME_ASSERT(Initialized);

		if (WalkingStacks)
		{
			BGSEECONSOLE->PrintToMessageLogContext(ConsoleMessageContext, "Couldn't reset stacks - Stack walk in progress");
			return;
		}

		ResetStack(UndoStack);
		ResetStack(RedoStack);
	}

	void BGSEEFormUndoStack::Print( const char* Format, ... )
	{
		SME_ASSERT(Initialized);

		char Buffer[0x512] = {0};

		va_list Args;
		va_start(Args, Format);
		vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
		va_end(Args);

		BGSEECONSOLE->PrintToMessageLogContext(ConsoleMessageContext, "%s", Buffer);
	}


}