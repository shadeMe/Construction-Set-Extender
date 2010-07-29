#include "Exports.h"
#include "BSAViewer.h"
#include "Common\MiscUtilities.h"

extern "C"{

__declspec(dllexport) const char* InitializeViewer(const char* AppPath, const char* Filter)
{
	return BSAV->InitializeViewer(gcnew String(AppPath), gcnew String(Filter));
}

}