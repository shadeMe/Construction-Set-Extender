#pragma once

#include <bgsee\FormUndoStack.h>

namespace cse
{
	namespace formUndoStack
	{
		class FormUndoStackOperator;

		class FormUndoProxy : public bgsee::FormUndoProxy
		{
		protected:
			TESForm*			Parent;
			TESForm*			Buffer;

			friend class FormUndoStackOperator;
		public:
			FormUndoProxy(TESForm* ParentForm, TESForm* CopySource = nullptr);
			virtual ~FormUndoProxy();

			virtual UInt32							GetFormID(void) const;
			virtual UInt8							GetType(void) const;
			virtual const char*						GetTypeString(void) const;
			virtual void							Undo(void);
		};

		class FormUndoStackOperator : public bgsee::FormUndoStackOperator
		{
		public:
			virtual ~FormUndoStackOperator();

			virtual bool							GetIsFormTypeUndoable(UInt8 Type);

			virtual bool							PreUndoRedoCallback(bgsee::FormUndoProxy* Proxy, bgsee::FormUndoProxy** OutAltProxy);
			virtual void							PostUndoRedoCallback(void);
		};

		void Initialize(void);
	}
}