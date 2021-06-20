#pragma once


namespace cse
{


namespace scriptEditor
{


delegate void ParameterizedActionDelegate(Object^ Param);
delegate void ActionDelegate();


generic <class TDelegate>
ref class Action
{
protected:
	String^ Name_;
	String^ Description_;
	TDelegate Delegate_;
public:
	Action(String^ Name, String^ Description);
	virtual ~Action();

	property String^ Name
	{
		String^ get() { return Name_; }
	}
	property String^ Description
	{
		String^ get() { return Description_; }
	}
	property TDelegate Delegate
	{
		TDelegate get() { return Delegate_; }
		void set(TDelegate v) { Delegate_ = v; }
	}
};


using BasicAction = Action<ActionDelegate^>;
using ParameterizedAction = Action<ParameterizedActionDelegate^>;


ref struct KeyCombo
{
	Keys Main;
	Keys Modifiers;

	void Validate();
public:
	KeyCombo(Keys Main, Keys Modifiers);

	property Keys Key
	{
		Keys get() { return Main; }
	}
	property bool Control
	{
		bool get() { return Modifiers.HasFlag(Keys::Control); }
	}
	property bool Shift
	{
		bool get() { return Modifiers.HasFlag(Keys::Shift); }
	}
	property bool Alt
	{
		bool get() { return Modifiers.HasFlag(Keys::Alt); }
	}

	bool IsTriggered(KeyEventArgs^ E);
	virtual bool Equals(Object^ obj) override;
	virtual int GetHashCode() override;
	virtual String^ ToString() override;

	static KeyCombo^ FromKeyEvent(KeyEventArgs^ E);
};


} // namespace scriptEditor


} // namespace cse