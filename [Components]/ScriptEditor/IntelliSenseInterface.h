#pragma once

#include "[Common]\AuxiliaryWindowsForm.h"
#include "ScriptTextEditorInterface.h"

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		ref class IntelliSenseItemVariable;

		ref class IntelliSenseInterfaceModel : public IIntelliSenseInterfaceModel
		{
			TextEditors::IScriptTextEditor^						ParentEditor;
			IIntelliSenseInterfaceView^							BoundParent;

			List<IntelliSenseItem^>^							LocalVariables;
			List<IntelliSenseItem^>^							EnumeratedItems;
			bool												CallingObjectIsRef;
			Script^												RemoteScript;
			IIntelliSenseInterfaceModel::Operation				LastOperation;

			property UInt32										PopupThresholdLength;
			property bool										UseSubstringFiltering;

			property bool										OverrideThresholdCheck;
			property bool										Enabled;

			void												ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);
			void												ParentEditor_KeyDown(Object^ Sender, TextEditors::IntelliSenseKeyEventArgs^ E);
			void												ParentEditor_ShowInterface(Object^ Sender, TextEditors::IntelliSenseShowEventArgs^ E);
			void												ParentEditor_HideInterface(Object^ Sender, TextEditors::IntelliSenseHideEventArgs^ E);
			void												ParentEditor_RelocateInterface(Object^ Sender, TextEditors::IntelliSensePositionEventArgs^ E);
			void												BoundParent_ItemSelected(Object^ Sender, EventArgs^ E);

			EventHandler^										ScriptEditorPreferencesSavedHandler;
			TextEditors::IntelliSenseKeyEventHandler^			ParentEditorKeyDown;
			TextEditors::IntelliSenseShowEventHandler^			ParentEditorShowInterface;
			TextEditors::IntelliSenseHideEventHandler^			ParentEditorHideInterface;
			TextEditors::IntelliSensePositionEventHandler^		ParentEditorRelocateInterface;
			EventHandler^										BoundParentItemSelectedHandler;

			property bool Bound
			{
				bool get() { return BoundParent != nullptr; }
			}

			void						Reset();
			void						EnumerateItem(IntelliSenseItem^ Item);
			void						PickSelection();
			void						PopulateDataStore(IIntelliSenseInterfaceModel::Operation O, bool IgnoreFilter);

			IntelliSenseItemVariable^	GetLocalVar(String^ Identifier);
		public:
			IntelliSenseInterfaceModel(TextEditors::IScriptTextEditor^ Parent);
			~IntelliSenseInterfaceModel();

#pragma region Interfaces
			virtual property List<IntelliSenseItem^>^ DataStore
			{
				virtual List<IntelliSenseItem^>^ get() { return EnumeratedItems; }
				virtual void set(List<IntelliSenseItem^>^ e) {}
			}

			virtual void				Bind(IIntelliSenseInterfaceView^ To);
			virtual void				Unbind();
			virtual	bool				GetTriggered(System::Windows::Input::Key E);
			virtual void				UpdateLocalVars(ObScriptSemanticAnalysis::AnalysisData^ Data);
#pragma endregion
		};

		ref class IntelliSenseInterfaceView : public IIntelliSenseInterfaceView
		{
			IIntelliSenseInterfaceModel^					AttachedModel;

			NonActivatingImmovableAnimatedForm^				Form;
			BrightIdeasSoftware::ObjectListView^			ListView;
			ToolTip^										Popup;

			property UInt32									MaximumVisibleItemCount;
			property bool									PreventActivation
			{
				virtual bool get() sealed { return Form->PreventActivation; }
				virtual void set(bool value) sealed { Form->PreventActivation = value; }
			}

			property bool Bound
			{
				bool get() { return AttachedModel != nullptr; }
			}

			void											ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);
			void											ListView_SelectionChanged(Object^ Sender, EventArgs^ E);
			void											ListView_ItemActivate(Object^ Sender, EventArgs^ E);
			void											ListView_KeyDown(Object^ Sender, KeyEventArgs^ E);

			static Object^									ListViewAspectGetter(Object^ RowObject);
			static Object^									ListViewImageGetter(Object^ RowObject);

			EventHandler^									ScriptEditorPreferencesSavedHandler;
			EventHandler^									ListViewSelectionChangedHandler;
			EventHandler^									ListViewItemActivateHandler;
			KeyEventHandler^								ListViewKeyDownHandler;

			void											DisplayToolTip(String^ Title,
																		   String^ Message,
																		   Point Location,
																		   IntPtr ParentHandle,
																		   UInt32 Duration);
			void											HideToolTip();
		public:
			IntelliSenseInterfaceView();
			~IntelliSenseInterfaceView();

#pragma region Interfaces
			virtual event EventHandler^		ItemSelected;

			property bool Visible
			{
				virtual bool get() { return Form->Visible; }
				virtual void set(bool e) {}
			}
			property IntelliSenseItem^ Selection
			{
				virtual IntelliSenseItem^ get() { return (IntelliSenseItem^)ListView->SelectedObject; }
				virtual void set(IntelliSenseItem^ e) {}
			}

			virtual void				Bind(IIntelliSenseInterfaceModel^ To);
			virtual void				Unbind();

			virtual void				ChangeSelection(IIntelliSenseInterfaceView::MoveDirection Direction);

			virtual void				Update();
			virtual void				Show(Point Location, IntPtr Parent);
			virtual void				Hide();
#pragma endregion
		};
	}
}
