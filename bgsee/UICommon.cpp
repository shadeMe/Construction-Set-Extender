#include "UICommon.h"

namespace bgsee
{
	WindowHandleCollection::HandleCollectionT::iterator WindowHandleCollection::Find( HWND Handle )
	{
		for (HandleCollectionT::iterator Itr = HandleList.begin(); Itr != HandleList.end(); Itr++)
		{
			if (*Itr == Handle)
				return Itr;
		}

		return HandleList.end();
	}

	bool WindowHandleCollection::Add( HWND Handle )
	{
		if (GetExists(Handle) == false)
		{
			HandleList.push_back(Handle);
			return true;
		}
		else
			return false;
	}

	bool WindowHandleCollection::Remove( HWND Handle )
	{
		HandleCollectionT::iterator Match = Find(Handle);
		if (Match != HandleList.end())
		{
			HandleList.erase(Match);
			return true;
		}
		else
			return false;
	}

	bool WindowHandleCollection::GetExists( HWND Handle )
	{
		return Find(Handle) != HandleList.end();
	}

	void WindowHandleCollection::Clear( void )
	{
		HandleList.clear();
	}

	WindowHandleCollection::WindowHandleCollection() :
		HandleList()
	{
		;//
	}

	WindowHandleCollection::~WindowHandleCollection()
	{
		Clear();
	}

	void WindowHandleCollection::SendMessage( UINT Msg, WPARAM wParam, LPARAM lParam )
	{
		for (const auto& Itr : HandleList)
			::SendMessage(Itr, Msg, wParam, lParam);
	}
}

