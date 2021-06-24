#pragma once


namespace cse
{


namespace scriptEditor
{


ref class IAction abstract
{
protected:
	String^ Name_;
	String^ Description_;
public:
	IAction(String^ Name, String^ Description);
	virtual ~IAction();

	property String^ Name
	{
		String^ get() { return Name_; }
	}
	property String^ Description
	{
		String^ get() { return Description_; }
	}

	virtual void Invoke() = 0;
};

ref class BasicAction : public IAction
{
public:
	delegate void InvokationDelegate();

	BasicAction(String^ Name, String^ Description);
	virtual ~BasicAction();

	property InvokationDelegate^ InvokeDelegate;

	virtual void Invoke() override;
};


ref struct KeyCombo
{
	Keys Main;
	Keys Modifiers;

	void Validate();

	KeyCombo(Keys Main, Keys Modifiers);
public:
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
	static KeyCombo^ New(Keys Modifier, Keys Key);
};


} // namespace scriptEditor


} // namespace cse