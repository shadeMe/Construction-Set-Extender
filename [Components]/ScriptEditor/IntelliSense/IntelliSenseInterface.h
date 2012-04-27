#pragma once

#include "[Common]\AuxiliaryWindowsForm.h"

namespace ConstructionSetExtender
{
	ref class NonActivatingImmovableAnimatedForm;

	namespace IntelliSense
	{
		ref class IntelliSenseItem;
		ref class IntelliSenseItemVariable;
		ref class Script;

		ref class IntelliSenseInterface
		{
		protected:
			void												IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
			void												IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void												IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);
			void												IntelliSenseBox_Cancel(Object^ Sender, CancelEventArgs^ E);

			static ToolTip^										InfoToolTip = gcnew ToolTip();

			IntelliSenseItemVariable^							LookupLocalVariableByIdentifier(String^% Identifier);
			bool												ShowQuickInfoTip(String^ MainToken, String^ ParentToken, Point Location);

			void												DisplayInfoToolTip(String^ Title, String^ Message, Point Location, IntPtr ParentHandle, UInt32 Duration);

			UInt32												ParentWorkspaceIndex;
			bool												Destroying;
			bool												CallingObjectIsRef;
			Script^												RemoteScript;

			List<IntelliSenseItem^>^							LocalVariableDatabase;

			ListView^											IntelliSenseList;
			NonActivatingImmovableAnimatedForm^					IntelliSenseBox;

			EventHandler^										IntelliSenseListSelectedIndexChangedHandler;
			KeyEventHandler^									IntelliSenseListKeyDownHandler;
			MouseEventHandler^									IntelliSenseListMouseDoubleClickHandler;
			CancelEventHandler^									IntelliSenseBoxCancelHandler;

			void												CleanupInterface();
			virtual void										Destroy();
		public:
			IntelliSenseInterface(UInt32 ParentWorkspaceIndex);
			virtual ~IntelliSenseInterface()
			{
				IntelliSenseInterface::Destroy();
			}

			static enum class									Operation
			{
				e_Default = 0,
				e_Call,
				e_Dot,
				e_Assign
			};

			static enum	class									MoveDirection
			{
				e_Up = 0,
				e_Down
			};

			property Operation									LastOperation;
			property bool										Enabled;
			property bool										Visible
			{
				virtual bool get() { return IntelliSenseBox->Visible; }
			}
			property UInt32										MaximumVisibleItemCount;
			property bool										PreventActivation
			{
				virtual bool get() { return IntelliSenseBox->PreventActivation; }
				virtual void set(bool value) { IntelliSenseBox->PreventActivation = value; }
			}

			virtual void										ShowInterface(IntelliSenseInterface::Operation DisplayOperation, bool ForceDisplay, bool ShowAllItems);
			virtual void										HideInterface();

			void												PickSelection();
			void												ChangeCurrentSelection(MoveDirection Direction);
			void												UpdateLocalVariableDatabase();
			bool												ShowQuickViewTooltip(String^ MainToken, String^ ParentToken);
			bool												ShowQuickViewTooltip(String^ MainToken, String^ ParentToken, Point MouseLocation);

			void												HideInfoToolTip();

			void												AddLocalVariableToDatabase(IntelliSenseItemVariable^ Variable);
			void												ClearLocalVariableDatabase();
		};
	}
}