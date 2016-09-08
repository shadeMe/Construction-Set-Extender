#pragma once

namespace cse
{
	namespace renderWindow
	{
		// provides logic for executing context specific operations
		class ExecutionContext
		{
		public:
			// render window modes in which the action can be executed
			enum
			{
				kMode_ReferenceEdit = 1 << 0,
				kMode_PathGridEdit = 1 << 1,
				kMode_LandscapeEdit = 1 << 2,

				kMode_All = kMode_ReferenceEdit | kMode_PathGridEdit | kMode_LandscapeEdit,
			};
		private:
			UInt8			Context;
		public:
			ExecutionContext(UInt8 Context);

			bool			operator==(const ExecutionContext& RHS);
			bool			operator!=(const ExecutionContext& RHS);

			bool			IsExecutable() const;			// returns true if the current render window mode matches the context

			bool			HasReferenceEdit() const;
			bool			HasPathGridEdit() const;
			bool			HasLandscapeEdit() const;
		};

		// functor that performs a render window action/operation
		class IRenderWindowAction
		{
		protected:
			std::string				Name;
			std::string				Description;
			ExecutionContext		Context;
		public:
			IRenderWindowAction(std::string Name, std::string Desc, UInt8 Context);
			virtual ~IRenderWindowAction() = 0;

			virtual bool					operator()() = 0;			// returns false if the operation was not performed, true otherwise
			virtual const char*				GetName() const;
			virtual const char*				GetDescription() const;
			virtual const ExecutionContext&	GetExecutionContext() const;
		};

		namespace input
		{
			// hardcoded stuff in the editor
			struct BuiltIn
			{
				enum
				{
					kModifier_Control = 1 << 0,
					kModifier_Shift = 1 << 1,
					kModifier_Alt = 1 << 2,

					// special modifiers
					kModifier_Space = 1 << 3,
					kModifier_Z = 1 << 4,
				};

				// keys that store their pressed state
				enum
				{
					kHoldable_Control = VK_CONTROL,
					kHoldable_Shift = VK_SHIFT,
					kHoldable_Space = VK_SPACE,
					kHoldable_X = 'X',
					kHoldable_Y = 'Y',
					kHoldable_Z = 'Z',
					kHoldable_S = 'S',
					kHoldable_V = 'V',
				};

				class ModifierOverride
				{
					enum
					{
						kKey_CTRL = 0,
						kKey_SHIFT = 1,
						kKey_ALT = 2,
						kKey_SPACE = 3,
						kKey_Z = 4,

						kKey__MAX,
					};

					struct State
					{
						UInt8*	BaseState;
						UInt8	StateBuffer;

						bool	Active;
						bool	NewState;
					};

					State		Data[kKey__MAX];
				public:
					ModifierOverride();
					~ModifierOverride();

					bool		IsActive(UInt8 Modifier) const;
					bool		GetOverrideState(UInt8 Modifier) const;

					void		Activate(UInt8 Modifier, bool NewState);
					void		Deactivate(UInt8 Modifier);

					static ModifierOverride		Instance;
				};

				class KeyBinding
				{
					UInt8		Modifiers;
					SHORT		KeyCode;
				public:
					KeyBinding(SHORT Key, UInt8 Modifiers = NULL);

					void		Trigger() const;
					bool		IsActivated(SHORT Key) const;

					SHORT		GetKeyCode() const;
					UInt8		GetModifiers(bool StripSpecialModifiers) const;
				};
			};

		}
	}
}