#pragma once
#include "RenderWindowCommon.h"
#include "RenderWindowActions.h"
#include "RenderWindowOSD.h"

/************************************************************************/
/*
built-in bindings:
-------------------------------------------------------------------------
key	(* - hold down)			action									context
===========================================================================
/************************************************************************/
/* WM_SYSKEYDOWN bindings
/************************************************************************
ctrl		f11				disable warnings
ctrl		f10				test all cells							ref
			f10				show scenegraph
alt			left			step selection anim back?				ref
alt			right			step selection anim forward?			ref
/************************************************************************/
/*
ctrl+shift	p				reload all pathgrids
			p				generate active exterior local map
			a				toggle bright light
			h				toggle landscape edit mode
			o				toggle vertex color paint mode (broken)	land
			f4				toggle collision geom
			f5				refresh render window
			up				move cam +Y axis
			down			move cam -Y axis
			right			move cam +X axis
			left			move cam -X axis
shift		up				pitch cam +
shift		down			pitch cam -
shift		left			yaw cam +
shift		right			yaw cam -
			home			roll cam +
			pg up			roll cam -
			i				show land use dialog					land
space		i				show object data info (broken)
ctrl+shift	r+space			show selected ref info					ref
			r				link path grid point selection			pathgrid
			m				toggle markers
shift		w				toggle water
			w				toggle global wireframe
shift		l				toggle landscape
			l				toggle light radius
			f				fall									ref pathgrid
ctrl		s				save plugin
ctrl		x				cut selection							ref
ctrl		y				redo									ref land
ctrl		z				undo									ref land
			backspace		undo									ref land
ctrl+alt	c				cull light selection					ref
shift		c				toggle objects
ctrl		c				auto-link path grid point selection		pathgrid
ctrl		c				copy selection							ref
			c+z				fall cam to landscape exterior
			c				center cam
			q				toggle vertex coloring mode				land
			t				top cam
ctrl		d				duplicate selection						ref
			d				deselect all							ref
			scroll lock		run havok sim							ref
			del				delete selection						ref pathgrid
			1				toggle selection wireframe				ref
ctrl+shift	b				check bounds
			b				toggle cell borders
ctrl		v				paste									ref
ctrl+shift	v				paste in place							ref
---------------------------------------------------------------------------------
ctrl*
shift*						rotate cam
			space*			pan cam
			v*				zoom cam
			x*				transform X axis						ref pathgrid
			y*				transform Y axis						ref pathgrid
			z*				transform Z axis						ref pathgrid
			s*				scale selection							ref
*/
/************************************************************************/

namespace cse
{
	namespace renderWindow
	{
		namespace input
		{
			class BasicKeyBinding
			{
				UInt8			Modifiers;
				SHORT			KeyCode;		// virtual key code
			public:
				BasicKeyBinding();
				BasicKeyBinding(SHORT Key, bool Control, bool Shift, bool Alt);
				BasicKeyBinding(SHORT Key, UInt8 Modifiers);
				BasicKeyBinding(const BuiltIn::KeyBinding& BuiltIn);
				BasicKeyBinding(const BasicKeyBinding& RHS);

				enum
				{
					kModifier_Control = BuiltIn::kModifier_Control,
					kModifier_Shift = BuiltIn::kModifier_Shift,
					kModifier_Alt = BuiltIn::kModifier_Alt,

					kModifier_CTRL_ALT = kModifier_Control | kModifier_Alt,
					kModifier_SHIFT_ALT = kModifier_Shift | kModifier_Alt,
					kModifier_CTRL_SHIFT = kModifier_Control | kModifier_Shift,
					kModifier_CTRL_SHIFT_ALT = kModifier_Control | kModifier_Shift | kModifier_Alt,
				};

				std::string		GetDescription() const;
				bool			IsActivated(SHORT Key = NULL) const;			// returns true if the key combo has been triggerred

				bool			IsValid() const;								// returns false if the main key code is invalid
				void			Set(SHORT Key, bool Control, bool Shift, bool Alt);
				void			Set(SHORT Key, UInt8 Modifiers);

				bool				operator==(const BasicKeyBinding& RHS);
				bool				operator!=(const BasicKeyBinding& RHS);
				BasicKeyBinding&	operator=(const BasicKeyBinding& RHS);

				bool			HasModifiers() const;
				bool			HasControl() const;
				bool			HasShift() const;
				bool			HasAlt() const;
				SHORT			GetKeyCode() const;

				static int				Serialize(const BasicKeyBinding& Combo);
				static BasicKeyBinding	Deserialize(int Serialized);
				static std::string		GetKeyName(SHORT Key);					// virtual key code
			};


			class IKeyboardEventHandler
			{
			public:
				enum
				{
					kType_Stateful		= 0,			// tracks key state (e.g., holdable keys)
					kType_Stateless		= 1,			// key state is only relevant at the time of handling (e.g., hotkeys)
				};
			public:
				virtual ~IKeyboardEventHandler() = 0;

				struct EventResult
				{
					bool	Triggered;			// true if the handler's key binding was activated
					bool	Success;			// true if the event was succcessfully handled
					bool	InvalidContext;		// true if the handler's execution context didn't match with the current context (not applicable to built-in events)

					EventResult() : Triggered(false), Success(false), InvalidContext(false) {}
				};

				virtual const ExecutionContext&		GetExecutionContext() const = 0;
				virtual const UInt8					GetHandlerType() const = 0;
				virtual EventResult					HandleActive(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;		// handle as a regular event
				virtual EventResult					HandleBuiltIn(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;		// handle as a built-in event
			};

			// a serializable/editable keybinding
			class IHotKey : public IKeyboardEventHandler
			{
			protected:
				std::string			GUID;
				BasicKeyBinding		DefaultBinding;
				BasicKeyBinding		ActiveBinding;
				ExecutionContext	Context;
				bool				Editable;

				virtual bool					IsActiveBindingTriggered(SHORT Key = NULL) const;
			public:
				IHotKey(const char* GUID, bool Editable = true, UInt8 Context = ExecutionContext::kMode_All);
				IHotKey(const char* GUID, bool Editable, const ExecutionContext& Context);
				virtual ~IHotKey() = 0;

				virtual const ExecutionContext&	GetExecutionContext() const override;

				virtual const char*				GetName() const = 0;
				virtual const char*				GetDescription() const = 0;

				virtual const BasicKeyBinding&	GetActiveBinding() const;
				virtual void					SetActiveBinding(const BasicKeyBinding& NewBinding);

				void							Save(bgsee::INIManagerSetterFunctor& INI, const char* Section) const;
				bool							Load(bgsee::INIManagerGetterFunctor& INI, const char* Section);		// returns false if unsuccessful

				bool							IsEditable() const;
			};

			class HoldableKeyHandler : public IHotKey
			{
			protected:
				std::string				Name;
				std::string				Description;

				virtual bool			IsActiveBindingTriggered(SHORT Key = NULL) const override;
			public:
				HoldableKeyHandler(const char* GUID, const char* Name, const char* Desc, BasicKeyBinding Default = BasicKeyBinding());
				inline virtual ~HoldableKeyHandler() override = default;

				virtual bool			IsHeldDown() const;		// returns true if the key is held down

				virtual const char*		GetName() const override;
				virtual const char*		GetDescription() const override;

				virtual void			SetActiveBinding(const BasicKeyBinding& NewBinding) override;

				virtual const UInt8		GetHandlerType() const override;
				virtual EventResult		HandleActive(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
				virtual EventResult		HandleBuiltIn(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
			};

			// overrides a built-in holdable key
			class HoldableKeyOverride : public HoldableKeyHandler
			{
			protected:
				SHORT				BuiltInKey;
			public:
				HoldableKeyOverride(const char* GUID, SHORT BuiltIn, bool Editable);
				inline virtual ~HoldableKeyOverride() override = default;

				UInt8*					GetBaseState() const;
				SHORT					GetBuiltInKey() const;

				virtual EventResult		HandleActive(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
				virtual EventResult		HandleBuiltIn(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
			};

			// overrides a built-in key combo
			class ComboKeyOverride : public IHotKey
			{
			protected:
				BuiltIn::KeyBinding				BuiltInCombo;
				actions::BuiltInKeyComboRWA&	BoundAction;
			public:
				ComboKeyOverride(const char* GUID, actions::BuiltInKeyComboRWA& Action);
				ComboKeyOverride(const char* GUID, actions::BuiltInKeyComboRWA& Action, BasicKeyBinding OverrideKey);
				inline virtual ~ComboKeyOverride() override = default;

				virtual const char*		GetName() const override;
				virtual const char*		GetDescription() const override;

				virtual const UInt8		GetHandlerType() const override;
				virtual EventResult		HandleActive(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
				virtual EventResult		HandleBuiltIn(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
			};

			class ActionableKeyHandler : public IHotKey
			{
			protected:
				IRenderWindowAction&	BoundAction;
			public:
				ActionableKeyHandler(const char* GUID, IRenderWindowAction& Action, BasicKeyBinding Default);
				inline virtual ~ActionableKeyHandler() override = default;

				virtual const char*		GetName() const override;
				virtual const char*		GetDescription() const override;

				virtual const UInt8		GetHandlerType() const override;
				virtual EventResult		HandleActive(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
				virtual EventResult		HandleBuiltIn(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
			};

			struct SharedBindings
			{
				const HoldableKeyHandler*	MoveCameraWithSelection;
			};

			class RenderWindowKeyboardManager
			{
				static const char*			kINISection_HotKeys;

				typedef std::vector<std::unique_ptr<IHotKey>>	HotKeyArrayT;
				typedef std::vector<BuiltIn::KeyBinding>		BuiltInKeyBindingArrayT;

				HotKeyArrayT				HotKeys;
				BuiltInKeyBindingArrayT		DeletedBindings;		// default combos that shouldn't be executed, e.g., broken bindings
				SharedBindings				Shared;
				void*						MessageLogContext;
				bool						Initialized;

				IHotKey*					LookupHotKey(BasicKeyBinding Key,
														 UInt8 HandlerType,
														 ExecutionContext Context = ExecutionContext(ExecutionContext::kMode_All),
														 bool OnlyMatchKeyCode = false);

				HoldableKeyOverride*		RegisterHoldableOverride(const char* GUID, SHORT HoldableKey, bool Editable);
				ComboKeyOverride*			RegisterComboKeyOverride(const char* GUID, actions::BuiltInKeyComboRWA& Action, BasicKeyBinding OverrideKey = BasicKeyBinding());
				ActionableKeyHandler*		RegisterActionableKeyHandler(const char* GUID, IRenderWindowAction& Action, BasicKeyBinding Default);
				HoldableKeyHandler*			RegisterHoldableHandler(const char* GUID, const char* Name, const char* Desc, BasicKeyBinding Default);

				void						SaveToINI() const;
				void						LoadFromINI();

				bool						RenderModalHotKeyEditor(RenderWindowOSD* OSD, ImGuiDX9* GUI);
				bool						RenderModalBindingEditor(RenderWindowOSD* OSD, ImGuiDX9* GUI, void* UserData);

				void						PerformConsistencyChecks(UINT uMsg, WPARAM wParam, LPARAM lParam);
			public:
				RenderWindowKeyboardManager();
				~RenderWindowKeyboardManager();

				void						Initialize();
				void						Deinitialize();

				bool						HandleInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, RenderWindowManager* Manager);		// returns true if input was handled/consumed
				void						ShowHotKeyEditor();
				const SharedBindings&		GetSharedBindings() const;
			};

			class RenderWindowMouseManager
			{
				enum
				{
					kSelectionPainting_NotSet = 0,
					kSelectionPainting_Select,
					kSelectionPainting_Deselect
				};

				POINT				CurrentMouseCoord;
				bool				PaintingSelection;
				UInt8				SelectionPaintingMode;
				POINT				MouseDownCursorPos;
				bool				FreeMouseMovement;

				POINT				CenterCursor(HWND hWnd, bool UpdateBaseCoords);			// returns the center coords (in screen area)
				bool				IsCenteringCursor(HWND hWnd, LPARAM lParam) const;
				void				GetWindowMetrics(HWND hWnd, int& X, int& Y, int& Width, int& Height) const;
			public:
				RenderWindowMouseManager();

				bool				HandleInput(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, RenderWindowManager* Manager);		// returns true if input was handled/consumed
				bool				IsPaintingSelection() const;
			};
		}
	}
}