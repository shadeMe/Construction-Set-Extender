#pragma once

#include "[Common]\AuxiliaryWindowsForm.h"
#include "..\SemanticAnalysis.h"
#include "..\ScriptTextEditorInterface.h"

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		ref class IntelliSenseItem;
		ref class IntelliSenseItemVariable;
		ref class Script;

		ref class IntelliSenseInterface
		{
		protected:
			static ToolTip^										InfoToolTip = gcnew ToolTip();

			void												IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
			void												IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void												IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);
			void												IntelliSenseList_RetrieveVirtualItem(Object^ Sender, RetrieveVirtualItemEventArgs^ E);
			void												IntelliSenseBox_Cancel(Object^ Sender, CancelEventArgs^ E);
			void												ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);

			void												DisplayToolTip(String^ Title, String^ Message, Point Location, IntPtr ParentHandle, UInt32 Duration);

			TextEditors::IScriptTextEditor^						ParentEditor;
			bool												DestructionFlag;
			bool												CallingObjectIsRef;
			Script^												RemoteScript;

			List<IntelliSenseItem^>^							LocalVariableDatabase;
			List<ListViewItem^>^								VirtualListCache;

			ListView^											IntelliSenseList;
			NonActivatingImmovableAnimatedForm^					IntelliSenseBox;

			EventHandler^										IntelliSenseListSelectedIndexChangedHandler;
			KeyEventHandler^									IntelliSenseListKeyDownHandler;
			MouseEventHandler^									IntelliSenseListMouseDoubleClickHandler;
			RetrieveVirtualItemEventHandler^					IntelliSenseListRetrieveVirtualItemEventHandler;
			CancelEventHandler^									IntelliSenseBoxCancelHandler;
			EventHandler^										ScriptEditorPreferencesSavedHandler;

			property UInt32										PopupThresholdLength;
			property UInt32										MaximumVisibleItemCount;
			property bool										PreventActivation
			{
				virtual bool get() { return IntelliSenseBox->PreventActivation; }
				virtual void set(bool value) { IntelliSenseBox->PreventActivation = value; }
			}
			property bool										UseSubstringFiltering;
			property bool										UseQuickView;

			void												Reset();
			void												EnumerateItem(IntelliSenseItem^ Item);
		public:
			IntelliSenseInterface(TextEditors::IScriptTextEditor^ Parent);
			~IntelliSenseInterface();

			static enum class									Operation
			{
				Default = 0,
				Call,
				Dot,
				Assign,
				Snippet,
			};

			static enum	class									MoveDirection
			{
				Up = 0,
				Down
			};

			property Operation									LastOperation;
			property bool										OverrideThresholdCheck;
			property bool										Enabled;
			property bool										Visible
			{
				virtual bool get() { return IntelliSenseBox->Visible; }
			}

			void												Show(IntelliSenseInterface::Operation DisplayOperation, bool ForceDisplay, bool ShowAllItems);
			void												Hide();
			void												MoveToCaret(bool AllowHidden);

			void												PickSelection();
			void												ChangeSelection(MoveDirection Direction);

			bool												ShowQuickViewTooltip(String^ MainToken, String^ ParentToken);
			bool												ShowQuickViewTooltip(String^ MainToken, String^ ParentToken, Point Location);
			void												HideQuickViewToolTip();

			IntelliSenseItemVariable^							LookupLocalVariableByIdentifier(String^ Identifier);
			void												UpdateLocalVariableDatabase(ObScriptSemanticAnalysis::AnalysisData^ Data);

			static bool											GetTriggered(System::Windows::Input::Key E);
		};
	}
}