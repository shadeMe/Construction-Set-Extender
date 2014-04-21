#include "Setting.h"

void Setting::SetStringValue( const char* Value )
{
	thisCall<void>(0x004C6780, this, Value);
}

UInt8 Setting::GetValueType( void )
{
	return cdeclCall<UInt8>(0x004C66D0, name);
}
