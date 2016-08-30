#pragma once

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowHotKeyManager
		{
			enum : UInt8
			{
				kKey_Shift		= 0,
				kKey_Control	= 1,
				kKey_Alt		= 2,

				kKey__MAX
			};

			struct KeyStateOverride
			{
				UInt8		Type;
				bool		Active;
				bool		OldState;
				bool		NewState;
			};

			KeyStateOverride	OverriddenKeyStates[kKey__MAX];

			void				SetKeyStateOverride(UInt8 Type, bool NewState);
			void				ResetKeyStateOverride(UInt8 Type);
		public:
			RenderWindowHotKeyManager();
			~RenderWindowHotKeyManager();


			SHORT				HandleGetAsyncKeyState(int vKey);
			void				SendDefaultHotKey(UInt8 VirtualKey, bool Shift, bool Control);
		};
	}
}