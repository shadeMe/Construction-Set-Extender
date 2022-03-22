#include "Crt.h"

void* Crt::EncodePointer(void* Ptr)
{
	return cdeclCall<void*>(0x00897DEB, Ptr);
}

void Crt::SetNewHandler(_PNH Handler)
{
	cdeclCall<void>(0x00897DBF, EncodePointer(Handler));
}

void Crt::SetPureCallHandler(_purecall_handler Handler)
{
	cdeclCall<void>(0x008989DF, EncodePointer(Handler));
}

void Crt::SetInvalidParameterHandler(_invalid_parameter_handler Handler)
{
	cdeclCall<void>(0x0088E5E6, EncodePointer(Handler));
}
