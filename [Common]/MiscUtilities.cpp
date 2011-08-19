#include "MiscUtilities.h"
#include "NativeWrapper.h"

CString::CString(System::String^% Source)
{
	P = Marshal::StringToHGlobalAnsi(Source);
}

void DebugDump(UInt8 Source, String^% Message)
{
	CString CStr(Message);
	g_CSEInterface->CSEEditorAPI.DebugPrint(Source, CStr.c_str());
}

void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State)
{
	if (State)	*Flag |= Mask;
	else		*Flag &= ~Mask;
}

ImageResourceManager::ImageResourceManager(String^ BaseName)
{
	Manager = gcnew ResourceManager(BaseName, Assembly::GetExecutingAssembly());
}

Image^ ImageResourceManager::CreateImageFromResource(String^ ResourceIdentifier)
{
	try
	{
		return dynamic_cast<Image^>(Manager->GetObject(ResourceIdentifier));
	}
	catch (...)
	{
		return nullptr;
	}
}

void ImageResourceManager::SetupImageForToolStripButton(ToolStripButton^ Control)
{
	Control->Image = CreateImageFromResource(Control->Name);
}

CSEControlDisposer::CSEControlDisposer( Control^% Source )
{
	for each (Control^ Itr in Source->Controls)
	{
		CSEControlDisposer Disposer(Itr);
		delete Itr;
	}
}

void DeleteManagedHeapPointer( void* Pointer, bool IsArray )
{
	if (IsArray)
		delete [] Pointer;
	else
		delete Pointer;
}

void CopyStringToCharBuffer( String^% Source, char* Buffer, UInt32 Size )
{
	int i = 0;
	for (i=0; i < Source->Length && i < Size; i++)
		*(Buffer + i) = Source[i];

	if (i < Size)
		*(Buffer + i) = '\0';
	else
		Buffer[Size - 1] = '\0';
}

void AnimatedForm::FadeTimer_Tick( Object^ Sender, EventArgs^ E )
{
	if (FadeOperation == FadeOperationType::e_FadeIn)
		this->Opacity += FadeTimer->Interval / (FadeAnimationFactor * FadeDuration * 1000);
	else
		this->Opacity -= FadeTimer->Interval / (FadeAnimationFactor * FadeDuration * 1000);

	if (this->Opacity >= 1.0 || this->Opacity <= 0.0)
	{
		FadeTimer->Stop();

		if (FadeOperation == FadeOperationType::e_FadeOut)
		{
			if (CloseOnFadeOut)
				Form::Close();
			else
				Form::Hide();
		}
	}
}

void AnimatedForm::Show()
{
	this->Opacity = 0.0;
	Form::Show();

	FadeOperation = FadeOperationType::e_FadeIn;
	FadeTimer->Start();
}

System::Windows::Forms::DialogResult AnimatedForm::ShowDialog()
{
	this->Opacity = 0.0;

	FadeOperation = FadeOperationType::e_FadeIn;
	FadeTimer->Start();

	return Form::ShowDialog();
}

void AnimatedForm::Hide()
{
	if (this->Visible)
	{
		FadeOperation = FadeOperationType::e_FadeOut;
		this->Opacity = 1.0;
		FadeTimer->Start();
	}
}

AnimatedForm::AnimatedForm( double FadeDuration ) : System::Windows::Forms::Form()
{
	this->FadeDuration = FadeDuration;
	FadeOperation = FadeOperationType::e_None;
	CloseOnFadeOut = false;

	FadeTimer = gcnew Timer();
	FadeTimer->Interval = 10;
	FadeTimer->Tick += gcnew EventHandler(this, &AnimatedForm::FadeTimer_Tick);
	FadeTimer->Stop();
}

void AnimatedForm::Close()
{
	CloseOnFadeOut = true;
	FadeOperation = FadeOperationType::e_FadeOut;
	this->Opacity = 1.0;
	FadeTimer->Start();
}

namespace Log
{
	namespace ScriptEditor
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung)
			{
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_SE, Message);
		}
	}
	namespace UseInfoList
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung)
			{
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_UL, Message);
		}
	}
	namespace BatchEditor
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung)
			{
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_BE, Message);
		}
	}
	namespace BSAViewer
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung)
			{
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_BSA, Message);
		}
	}
	namespace TagBrowser
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung)
			{
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_TAG, Message);
		}
	}
}