#pragma once

#include <BGSEEFormUndoStack.h>

namespace ConstructionSetExtender
{
	namespace FormUndoStack
	{
		class CSEFormUndoStackOperator;

		class CSEFormUndoProxy : public BGSEditorExtender::BGSEEFormUndoProxy
		{
		protected:
			TESForm*			Parent;
			TESForm*			Buffer;

			friend class CSEFormUndoStackOperator;
		public:
			CSEFormUndoProxy(TESForm* ParentForm, TESForm* CopySource = NULL);
			virtual ~CSEFormUndoProxy();

			virtual UInt32							GetFormID(void) const;
			virtual UInt8							GetType(void) const;
			virtual const char*						GetTypeString(void) const;
			virtual void							Undo(void);
		};

		class CSEFormUndoStackOperator : public BGSEditorExtender::BGSEEFormUndoStackOperator
		{
		public:
			virtual ~CSEFormUndoStackOperator();

			virtual bool							GetIsFormTypeUndoable(UInt8 Type);

			virtual bool							PreUndoRedoCallback(BGSEditorExtender::BGSEEFormUndoProxy* Proxy, BGSEditorExtender::BGSEEFormUndoProxy** OutAltProxy);
			virtual void							PostUndoRedoCallback(void);
		};

		void Initialize(void);
	}
}