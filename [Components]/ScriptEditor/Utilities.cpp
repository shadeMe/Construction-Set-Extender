#include "Utilities.h"

namespace cse
{


namespace scriptEditor
{


BasicAction::BasicAction(String^ Name, String^ Description)
	: IAction(Name, Description)
{
}

BasicAction::~BasicAction()
{
	delete InvokeDelegate;
}

void BasicAction::Invoke()
{
	InvokeDelegate();
}

void KeyCombo::Validate()
{
	if (Main.HasFlag(Keys::Control) || Main.HasFlag(Keys::Shift) || Main.HasFlag(Keys::Alt))
		throw gcnew InvalidEnumArgumentException();
}

KeyCombo::KeyCombo(Keys Main, Keys Modifiers) : Main(Main), Modifiers(Modifiers)
{
	Validate();
}

bool KeyCombo::IsTriggered(KeyEventArgs^ E)
{
	if (Control && !E->Control)
		return false;
	else if (Shift && !E->Shift)
		return false;
	else if (Alt && !E->Alt)
		return false;

	return Main == E->KeyCode;
}

bool KeyCombo::Equals(Object^ obj)
{
	if (obj == nullptr)
		return false;
	else if (obj->GetType() != KeyCombo::typeid)
		return false;

	auto Other = safe_cast<KeyCombo^>(obj);
	return this->Main.Equals(Other->Main) && this->Modifiers.Equals(Other->Modifiers);
}

int KeyCombo::GetHashCode()
{
	int Hash = 7;
	Hash = 31 * Hash + static_cast<int>(Main);
	Hash = 31 * Hash + static_cast<int>(Modifiers);
	return Hash;
}


System::String^ KeyCombo::ToString()
{
	return Modifiers.ToString() + " + " + Main.ToString();
}

KeyCombo^ KeyCombo::FromKeyEvent(KeyEventArgs^ E)
{
	UInt32 Modifiers;
	if (E->Control)
		Modifiers |= safe_cast<UInt32>(Keys::Control);
	if (E->Shift)
		Modifiers |= safe_cast<UInt32>(Keys::Shift);
	if (E->Alt)
		Modifiers |= safe_cast<UInt32>(Keys::Alt);

	return gcnew KeyCombo(E->KeyCode, safe_cast<Keys>(Modifiers));
}

KeyCombo^ KeyCombo::New(Keys Modifier, Keys Key)
{
	return gcnew KeyCombo(Key, Modifier);
}


} // namespace scriptEditor


} // namespace cse