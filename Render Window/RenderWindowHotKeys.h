#pragma once
#include "RenderWindowActions.h"
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowHotKeyManager;

		class KeyCombo
		{
			UInt8			Modifiers;
			SHORT			KeyCode;		// virtual key code
		public:
			KeyCombo();
			KeyCombo(SHORT Key, bool Control, bool Shift, bool Alt);
			KeyCombo(SHORT Key, UInt8 Modifiers);
			KeyCombo(const KeyCombo& RHS);

			enum
			{
				kModifier_Control			= 1 << 0,
				kModifier_Shift				= 1 << 1,
				kModifier_Alt				= 1 << 2,

				kModifier_CTRL_ALT			= kModifier_Control | kModifier_Alt,
				kModifier_CTRL_SHIFT		= kModifier_Control | kModifier_Shift,
				kModifier_CTRL_SHIFT_ALT	= kModifier_Control | kModifier_Shift | kModifier_Alt,
			};

			std::string		GetDescription() const;
			bool			IsActivated(SHORT Key) const;					// returns true if the key combo has been triggerred

			bool			IsValid() const;								// returns false if the main key code is invalid
			void			Set(SHORT Key, bool Control, bool Shift, bool Alt);
			void			Set(SHORT Key, UInt8 Modifiers);

			bool			operator==(const KeyCombo& RHS);
			KeyCombo&		operator=(const KeyCombo& RHS);

			bool			HasControl() const;
			bool			HasShift() const;
			bool			HasAlt() const;

			static int				Serialize(const KeyCombo& Combo);
			static KeyCombo			Deserialize(int Serialized);
			static std::string		GetKeyName(SHORT Key);
		};

		class RenderWindowHotKey
		{
			friend class RenderWindowHotKeyManager;

			std::string						GUID;
			KeyCombo						ActiveKey;
			KeyCombo						DefaultKey;					// as init'd by the manager
			KeyCombo						BuiltinKey;					// the built-in key combo, if any
			actions::IRenderWindowAction&	BoundAction;
		public:
			RenderWindowHotKey(const char* GUID, actions::IRenderWindowAction& Action, KeyCombo Active);
			RenderWindowHotKey(const char* GUID, actions::IRenderWindowAction& Action, KeyCombo Active, KeyCombo Builtin);

			bool							HandleActiveKeyCombo(SHORT Key, bool& OutActionResult, bool& OutIsInvalidContext);		// returns true if the key combo was triggered
			bool							HandleBuiltinKeyCombo(SHORT Key);		// returns true if the built-in key combo was triggered
			bool							IsOverride() const;						// returns true if the hotkey overrides a built-in key combo
			void							SetActiveCombo(SHORT Key, UInt8 Modifiers);
			void							Save(bgsee::INIManagerSetterFunctor& INI, const char* Section) const;
			bool							Load(bgsee::INIManagerGetterFunctor& INI, const char* Section);		// returns false if unsuccessful
			const char*						GetName() const;
			const char*						GetDescription() const;
		};

		class RenderWindowHotKeyManager
		{
			static const char*			kINISection;
			enum
			{
				kKey_Shift		= 0,
				kKey_Control	= 1,
				kKey_Alt		= 2,

				kKey__MAX
			};

			// when tunnelling messages to the base wnd proc
			struct KeyStateOverride
			{
				UInt8		Type;
				bool		Active;
				bool		OldState;
				bool		NewState;
			};

			typedef std::vector<RenderWindowHotKey>			HotKeyArrayT;

			HotKeyArrayT		HotKeys;
			KeyStateOverride	OverriddenKeyStates[kKey__MAX];
			bool				Initialized;

			void				SetKeyStateOverride(UInt8 Type, bool NewState);
			void				ResetKeyStateOverride(UInt8 Type);

			RenderWindowHotKey*	LookupHotKey(KeyCombo Key);		// returns nullptr if nothing's found,
			RenderWindowHotKey*	LookupHotKey(KeyCombo Key, actions::IRenderWindowAction& Action);	// filters with the action's execution context
			void				RegisterHotKey(const char* GUID, actions::IRenderWindowAction& Action, KeyCombo& Combo);
			void				RegisterHotKey(const char* GUID, actions::IRenderWindowAction& Action, KeyCombo& Combo, KeyCombo& Default);

			void				SaveToINI() const;
			void				LoadFromINI();

			bool				RenderModalHotKeyEditor(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			bool				RenderModalBindingEditor(RenderWindowOSD* OSD, ImGuiDX9* GUI, void* UserData);
		public:
			RenderWindowHotKeyManager();
			~RenderWindowHotKeyManager();

			void				Initialize();
			void				Deinitialize();

			bool				HandleInput(UINT uMsg, WPARAM wParam, LPARAM lParam);		// returns true if input was handled/consumed
			void				ShowHotKeyEditor();
			void				SendBuiltinHotKey(SHORT Key, bool Shift, bool Control);

			SHORT				HandleDefaultWndProcAsyncKeyState(int vKey);
		};
	}
}