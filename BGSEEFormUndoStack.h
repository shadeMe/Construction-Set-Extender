#pragma once

#include "BGSEEWrappers.h"

namespace BGSEditorExtender
{
	class BGSEEFormUndoStack;

	class BGSEEFormUndoProxy
	{
	protected:
		static int							GIC;

		std::string							EditorID;			// editorID cache
		bool								Modified;			// form's modified flag state

		friend class BGSEEFormUndoStack;
	public:
		BGSEEFormUndoProxy();
		virtual ~BGSEEFormUndoProxy();

		const char*											GetEditorID(void) const;
		virtual UInt32										GetFormID(void) const = 0;
		virtual UInt8										GetType(void) const = 0;
		virtual const char*									GetTypeString(void) const = 0;

															// copies the buffer's data to the parent and performs any necessary fix ups
		virtual void										Undo(void) = 0;
	};

	class BGSEEFormUndoStackOperator
	{
	public:
		virtual ~BGSEEFormUndoStackOperator() = 0
		{
			;//
		}

		virtual bool										GetIsFormTypeUndoable(UInt8 Type) = 0;

															// called before an undo/redo op
															// returns false if the proxy is invalidated in any way (parent form is no longer valid, etc)
															// if false, proxy will be removed from the stack and the operation canceled
															// if true, second param must be filled with a proxy that has the current form data (manager takes ownership of the pointer)
		virtual bool										PreUndoRedoCallback(BGSEEFormUndoProxy* Proxy, BGSEEFormUndoProxy** OutAltProxy) = 0;
		virtual void										PostUndoRedoCallback(void) = 0;
	};

	class BGSEEFormUndoStack
	{
		static BGSEEFormUndoStack*			Singleton;
		static const char*					kMessageLogContextName;

		BGSEEFormUndoStack();

		typedef boost::shared_ptr<BGSEEFormUndoProxy>		UndoProxyHandleT;
		typedef std::stack<UndoProxyHandleT>				UndoProxyStackT;

		BGSEEFormUndoStackOperator*			Operator;
		UndoProxyStackT						UndoStack;
		UndoProxyStackT						RedoStack;
		void*								ConsoleMessageContext;
		bool								WalkingStacks;

		bool								Initialized;

		enum
		{
			kOperation_Undo = 0,
			kOperation_Redo
		};

		void								ResetStack(UndoProxyStackT& Stack);
		void								WalkUndoStack(UInt8 Operation, UndoProxyStackT& Stack, UndoProxyStackT& AlternateStack);
	public:
		~BGSEEFormUndoStack();

		static BGSEEFormUndoStack*			GetSingleton(void);

		bool								Initialize(BGSEEFormUndoStackOperator* Operator);			// takes ownership of the pointer

											// takes ownership of the pointer, automatically resets the redo stack
											// returns true if the proxy was recorded successfully, false otherwise
		bool								Record(BGSEEFormUndoProxy* Proxy);

		void								PerformUndo(void);
		void								PerformRedo(void);

		void								Reset(void);												// release all proxies and clear the stacks
		void								Print(const char* Format, ...);
	};

#define BGSEEUNDOSTACK						BGSEditorExtender::BGSEEFormUndoStack::GetSingleton()
}