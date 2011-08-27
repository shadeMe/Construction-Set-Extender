#pragma once
#include "Includes.h"

ref class CString
{
	IntPtr												P;

	void												Free() { Marshal::FreeHGlobal(P); }
public:
	CString(String^% Source);

	~CString()											{ this->!CString(); }
	!CString()											{ this->Free(); }

	const char*											c_str() { return static_cast<char*>(P.ToPointer()); }
};

void													DebugDump(UInt8 Source, String^% Message);
void													ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]

ref class CSEGeneralException : public Exception
{
public:
	CSEGeneralException(String^ Message) : Exception(Message) {};
};

ref class ImageResourceManager
{
	ResourceManager^					Manager;
public:
	ImageResourceManager(String^ BaseName);

	Image^								CreateImageFromResource(String^ ResourceIdentifier);
	void								SetupImageForToolStripButton(ToolStripButton^ Control);
};

ref class CSEControlDisposer
{
public:
	CSEControlDisposer(Control^% Source);
};

ref class WindowHandleWrapper : public IWin32Window
{
	IntPtr					_hwnd;
public:
	WindowHandleWrapper(IntPtr Handle) : _hwnd(Handle) {}

    property IntPtr Handle
    {
        virtual IntPtr get()
        {
            return _hwnd;
        }
    };
};

void									DeleteManagedHeapPointer(void* Pointer, bool IsArray);
void									CopyStringToCharBuffer(String^% Source, char* Buffer, UInt32 Size);

ref class AnimatedForm : public System::Windows::Forms::Form
{
protected:
	static enum class									FadeOperationType
																{
																	e_None = 0,
																	e_FadeIn,
																	e_FadeOut
																};

	FadeOperationType					FadeOperation;
	Timer^								FadeTimer;
	double								FadeDuration;
	bool								CloseOnFadeOut;

	static double						FadeAnimationFactor = 0.60;

	void								FadeTimer_Tick(Object^ Sender, EventArgs^ E);
public:
	AnimatedForm(double FadeDuration);
	~AnimatedForm()
	{
		FadeTimer->Stop();
	}

	void										Show();
	void										Show(IWin32Window^ Parent);
	System::Windows::Forms::DialogResult		ShowDialog();
	void										Hide();
	void										Close();
};

ref class NonActivatingImmovableAnimatedForm : public Form
{
protected:
	property bool										ShowWithoutActivation
	{
		virtual bool									get() override { return true; }
	}

	virtual void										WndProc(Message% m) override;

	static enum class									FadeOperationType
															{
																e_None = 0,
																e_FadeIn,
																e_FadeOut
															};

	bool												AllowMove;
	FadeOperationType									FadeOperation;
	Timer^												FadeTimer;

	void												FadeTimer_Tick(Object^ Sender, EventArgs^ E);

	void												Destroy();
public:
	~NonActivatingImmovableAnimatedForm()
	{
		Destroy();
	}

	void												SetSize(Drawing::Size WindowSize);
	void												ShowForm(Drawing::Point Position, IntPtr ParentHandle, bool Animate);
	void												HideForm(bool Animate);

	NonActivatingImmovableAnimatedForm();
};

namespace Log
{
	enum MessageSource
	{
		e_CSE = 0,
		e_CS,
		e_BE,
		e_UL,
		e_SE,
		e_BSA,
		e_TAG
	};

	namespace ScriptEditor
	{
		void DebugPrint(String^ Message, bool Achtung = false);
	}
	namespace UseInfoList
	{
		void DebugPrint(String^ Message, bool Achtung = false);
	}
	namespace BatchEditor
	{
		void DebugPrint(String^ Message, bool Achtung = false);
	}
	namespace BSAViewer
	{
		void DebugPrint(String^ Message, bool Achtung = false);
	}
	namespace TagBrowser
	{
		void DebugPrint(String^ Message, bool Achtung = false);
	}
}