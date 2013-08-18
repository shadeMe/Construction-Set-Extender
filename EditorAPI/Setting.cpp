#include "Setting.h"

void Setting::SetStringValue( const char* Value )
{
	thisCall<void>(0x004C6780, this, Value);
}