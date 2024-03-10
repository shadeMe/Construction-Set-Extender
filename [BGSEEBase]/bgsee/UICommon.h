#pragma once

namespace bgsee
{
	typedef SInt32 ResourceTemplateOrdinalT;

	// Actual resource template ordinals used in the editor are always positive
	// So, we can use a negative value as a sentinel
	static constexpr ResourceTemplateOrdinalT	kInvalidResourceTemplateOrdinal = -1;

	class WindowHandleCollection
	{
		typedef std::vector<HWND>	HandleCollectionT;
		HandleCollectionT			HandleList;

		HandleCollectionT::iterator	Find(HWND Handle);
	public:
		WindowHandleCollection();
		~WindowHandleCollection();

		bool	Add(HWND Handle);
		bool	Remove(HWND Handle);
		bool	GetExists(HWND Handle);
		void	Clear(void);

		void	SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	};

	std::string WindowMessageToString(UINT uMsg, bool AppendHexValue = true);
}