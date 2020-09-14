#pragma once

#include "[Common]\AuxiliaryWindowsForm.h"
#include "IIntelliSenseInterface.h"

namespace cse
{
	namespace intellisense
	{
		ref class IntelliSenseInterfaceView : public IIntelliSenseInterfaceView
		{
			IIntelliSenseInterfaceModel^			BoundModel;

			NonActivatingImmovableAnimatedForm^		Form;
			BrightIdeasSoftware::ObjectListView^	ListView;
			ToolTip^								ListViewPopup;
			ToolTip^								InsightPopup;

			property UInt32							MaximumVisibleItemCount;
			property bool							PreventActivation
			{
				virtual bool get() sealed { return Form->PreventActivation; }
				virtual void set(bool value) sealed { Form->PreventActivation = value; }
			}
			property UInt32							InsightPopupDisplayDuration;

			property bool Bound
			{
				bool get() { return BoundModel != nullptr; }
			}

			void				ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);
			void				ListView_SelectionChanged(Object^ Sender, EventArgs^ E);
			void				ListView_ItemActivate(Object^ Sender, EventArgs^ E);
			void				ListView_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void				ListView_KeyUp(Object^ Sender, KeyEventArgs^ E);
			void				ListView_FormatRow(Object^ Sender, BrightIdeasSoftware::FormatRowEventArgs^ E);

			static Object^		ListViewAspectGetter(Object^ RowObject);
			static Object^		ListViewImageGetter(Object^ RowObject);

			EventHandler^		ScriptEditorPreferencesSavedHandler;
			EventHandler^		ListViewSelectionChangedHandler;
			EventHandler^		ListViewItemActivateHandler;
			KeyEventHandler^	ListViewKeyDownHandler;
			KeyEventHandler^	ListViewKeyUpHandler;
			EventHandler<BrightIdeasSoftware::FormatRowEventArgs^>^
								ListViewFormatRowHandler;

			void				ShowListViewToolTip(String^ Title,
											String^ Message,
											Point Location,
											IntPtr ParentHandle,
											UInt32 Duration);
			void				ShowListViewToolTip(IntelliSenseItem^ Item);
			void				HideListViewToolTip();

			delegate void		UIInvokeDelegate_FormShow(IntelliSenseInterfaceView^, NonActivatingImmovableAnimatedForm^, Point, IntPtr);
			delegate void		UIInvokeDelegate_FormSetSize(IntelliSenseInterfaceView^, NonActivatingImmovableAnimatedForm^, Size);
			delegate void		UIInvokeDelegate_FormHide(IntelliSenseInterfaceView^, NonActivatingImmovableAnimatedForm^);

			static void			UIInvoke_FormShow(IntelliSenseInterfaceView^ Sender, NonActivatingImmovableAnimatedForm^ ToInvoke, Point Location, IntPtr Parent);
			static void			UIInvoke_FormSetSize(IntelliSenseInterfaceView^ Sender, NonActivatingImmovableAnimatedForm^ ToInvoke, Size ToSet);
			static void			UIInvoke_FormHide(IntelliSenseInterfaceView^ Sender, NonActivatingImmovableAnimatedForm^ ToInvoke);

			static const float	DimmedOpacity = 0.1f;
		public:
			IntelliSenseInterfaceView();
			~IntelliSenseInterfaceView();

#pragma region Interfaces
			virtual event EventHandler^		ItemSelected;
			virtual event EventHandler^		Dismissed;

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
			virtual void				DimOpacity();
			virtual void				ResetOpacity();

			virtual void				ShowInsightToolTip(IntelliSenseShowInsightToolTipArgs^ Args);
			virtual void				HideInsightToolTip();

			virtual void				Update();
			virtual void				Show(Point Location, IntPtr Parent);
			virtual void				Hide();
#pragma endregion
		};
	}
}