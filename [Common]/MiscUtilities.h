#pragma once
#include "IncludesCLR.h"

#pragma warning(disable: 4677)

namespace cse
{
	ref class CString
	{
		IntPtr	P;

		void		Free() { Marshal::FreeHGlobal(P); }
	public:
		CString(String^ Source);
		~CString();

		const char* c_str() { return static_cast<char*>(P.ToPointer()); }
	};

	void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]

	ref class CSEGeneralException : public System::InvalidOperationException
	{
	public:
		CSEGeneralException(String^ Message) : System::InvalidOperationException(Message) {};
	};

	ref class ImageResourceManager
	{
		ResourceManager^	Manager;
	public:
		ImageResourceManager(String^ BaseName);

		Image^				CreateImage(String^ ResourceIdentifier);
		void				SetupImageForToolStripButton(ToolStripButton^ Control);
	};

	ref class CSEControlDisposer
	{
	public:
		CSEControlDisposer(Control^ Source);
		CSEControlDisposer(Control::ControlCollection^ Source);
	};

	ref class WindowHandleWrapper : public IWin32Window
	{
		IntPtr					_hwnd;
	public:
		WindowHandleWrapper(IntPtr Handle) : _hwnd(Handle) {}

		property IntPtr Handle
		{
			virtual IntPtr get()
			{
				return _hwnd;
			}
		};
	};

	ref class LineTrackingStringReader : public System::IO::StringReader
	{
		UInt32					CurrentLine;
	public:
		LineTrackingStringReader(String^% In) : StringReader(In), CurrentLine(0) {}

		virtual String^			ReadLine() override
		{
			String^ Out = StringReader::ReadLine();
			if (Out != nullptr)
				CurrentLine++;

			return Out;
		}

		property UInt32			LineNumber			// line no of the last read line
		{
			virtual UInt32 get() { return CurrentLine; }
		}
	};

	void CopyStringToCharBuffer(String^% Source, char* Buffer, UInt32 Size);

	generic<typename T>
	ref class SimpleBindingList : public System::Collections::IEnumerable
	{
	public:
		ref struct AddRemoveEventArgs
		{
			T		Item;
			bool	UpdateInProgress;

			AddRemoveEventArgs(T Item, bool Updating) : Item(Item), UpdateInProgress(Updating) {}
		};

		ref struct ClearEventArgs
		{
			bool	UpdateInProgress;

			ClearEventArgs(bool Updating) : UpdateInProgress(Updating) {}
		};

		ref struct UpdateEventArgs
		{
			UInt32	Count;		// number of items in the list

			UpdateEventArgs(UInt32 Count) : Count(Count) {}
		};

		ref struct SortEventArgs
		{
			System::Collections::Generic::IComparer<T>^		Comparer;

			SortEventArgs(System::Collections::Generic::IComparer<T>^ Comparer) : Comparer(Comparer) {}
		};

		delegate void	AddRemoveEventHandler(Object^ Sender, AddRemoveEventArgs^ E);
		delegate void	ClearEventHandler(Object^ Sender, ClearEventArgs^ E);
		delegate void	UpdateEventHandler(Object^ Sender, UpdateEventArgs^ E);
		delegate void	SortEventHandler(Object^ Sender, SortEventArgs^ E);
	protected:
		List<T>^	DataStore;
		bool		Updating;

		void		OnAdd(T Item) { Added(this, gcnew AddRemoveEventArgs(Item, Updating)); }
		void		OnRemove(T Item) { Removed(this, gcnew AddRemoveEventArgs(Item, Updating)); }
		void		OnClear() { Cleared(this, gcnew ClearEventArgs(Updating)); }
		void		OnBeginUpdate(UInt32 Count) { UpdateStarted(this, gcnew UpdateEventArgs(Count)); }
		void		OnEndUpdate(UInt32 Count) { UpdateStopped(this, gcnew UpdateEventArgs(Count)); }
		void		OnSorted(System::Collections::Generic::IComparer<T>^ Comparer) { Sorted(this, gcnew SortEventArgs(Comparer)); }
	public:
		SimpleBindingList() : DataStore(gcnew List<T>), Updating(false) {}
		~SimpleBindingList()
		{
			Clear();
		}

		event AddRemoveEventHandler^	Added;
		event AddRemoveEventHandler^	Removed;
		event ClearEventHandler^		Cleared;
		event UpdateEventHandler^		UpdateStarted;
		event UpdateEventHandler^		UpdateStopped;
		event SortEventHandler^			Sorted;

		void Add(T Item)
		{
			DataStore->Add(Item);
			OnAdd(Item);
		}

		void Remove(T Item)
		{
			DataStore->Remove(Item);
			OnRemove(Item);
		}

		void Clear()
		{
			DataStore->Clear();
			OnClear();
		}

		void BeginUpdate()
		{
			if (Updating)
				throw gcnew System::InvalidOperationException("An update is already in progress");

			Updating = true;
			OnBeginUpdate(DataStore->Count);
		}

		void EndUpdate(bool Sort, System::Collections::Generic::IComparer<T>^ Comparer)
		{
			if (Updating == false)
				throw gcnew System::InvalidOperationException("No update in progress");

			Updating = false;
			OnEndUpdate(DataStore->Count);

			if (Sort)
				this->Sort(Comparer);
		}

		bool Contains(T Item)
		{
			return DataStore->Contains(Item);
		}

		property UInt32 Count
		{
			virtual UInt32 get() { return DataStore->Count; }
			virtual void set(UInt32 e) { throw gcnew System::InvalidOperationException; }
		}

		virtual System::Collections::IEnumerator^ GetEnumerator()
		{
			return DataStore->GetEnumerator();
		}

		void Sort()
		{
			DataStore->Sort();
			OnSorted(nullptr);
		}

		void Sort(System::Collections::Generic::IComparer<T>^ Comparer)
		{
			DataStore->Sort(Comparer);
			OnSorted(Comparer);
		}
	};

	// Fix for the white bottom border when using the System toolstrip renderer
	// https://stackoverflow.com/questions/1918247/how-to-disable-the-line-under-tool-strip-in-winform-c
	ref class CustomToolStripSystemRenderer : public ToolStripSystemRenderer
	{
	protected:
		virtual void OnRenderToolStripBorder(ToolStripRenderEventArgs^ e) override {}
	};
}