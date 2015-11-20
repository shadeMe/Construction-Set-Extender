#pragma once

#include "SME_Prefix.h"

#pragma warning(disable : 4800 4244 4390 4018 4570 4091)

#using <mscorlib.dll>
#using <System.DLL>
#using <System.Drawing.DLL>
#using <System.Windows.Forms.DLL>

using namespace cli;
using namespace System;
using namespace System::ComponentModel;
using namespace System::Diagnostics;

using namespace System::IO;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

using namespace System::Resources;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

typedef unsigned char		UInt8;

#define		SCRIPTEDITOR_TITLE			"CSE Script Editor"
#define		COMPONENTDLLFOLDER			"Data\\OBSE\\Plugins\\CSE\\"

#define SAFEDELETE_CLR(Identifier)		delete Identifier; Identifier = nullptr