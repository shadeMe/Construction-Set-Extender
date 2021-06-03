#pragma once

#include "[Common]\AuxiliaryWindowsForm.h"
#include "IIntelliSenseInterface.h"

namespace cse
{
	namespace intellisense
	{
		using namespace DevComponents;

		ref class IntelliSenseInterfaceView : public IIntelliSenseInterfaceView
		{
			static enum class FormInvokeDelegate
			{
				Show, SetSize, Hide
			};

			IIntelliSenseInterfaceModel^			BoundModel;

			AnimatedForm^							Form;
			BrightIdeasSoftware::ObjectListView^	ListView;
			DotNetBar::SuperTooltip^				ListViewPopup;
			DotNetBar::SuperTooltip^				InsightPopup;
			ImageList^								IntelliSenseItemImages;

			property UInt32							MaximumVisibleItemCount;
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

			void				ShowListViewToolTip(IntelliSenseItem^ Item);
			void				HideListViewToolTip();

			delegate void		UIInvokeDelegate_FormShow(IntelliSenseInterfaceView^, AnimatedForm^, Point, IntPtr);
			delegate void		UIInvokeDelegate_FormSetSize(IntelliSenseInterfaceView^, AnimatedForm^, Size);
			delegate void		UIInvokeDelegate_FormHide(IntelliSenseInterfaceView^, AnimatedForm^);

			static void			UIInvoke_FormShow(IntelliSenseInterfaceView^ Sender, AnimatedForm^ ToInvoke, Point Location, IntPtr Parent);
			static void			UIInvoke_FormSetSize(IntelliSenseInterfaceView^ Sender, AnimatedForm^ ToInvoke, Size ToSet);
			static void			UIInvoke_FormHide(IntelliSenseInterfaceView^ Sender, AnimatedForm^ ToInvoke);

			static const float	DimmedOpacity = 0.1f;
		public:
			IntelliSenseInterfaceView();
			~IntelliSenseInterfaceView();

#pragma region Interfaces
			virtual event EventHandler^		ItemSelected;
			virtual event EventHandler^		Dismissed;

			property bool Visible
			{
				virtual bool get()
				{
					if (Form->IsFadingIn)
						return true;
					else if (Form->Tag != nullptr && safe_cast<FormInvokeDelegate>(Form->Tag) == FormInvokeDelegate::Show)
						return false;
					else if (Form->IsFadingOut)
						return false;
					else if (Form->Tag != nullptr && safe_cast<FormInvokeDelegate>(Form->Tag) == FormInvokeDelegate::Hide)
						return false;
					else
						return Form->Visible;
				}
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