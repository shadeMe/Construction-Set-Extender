#pragma once

// Container class to encapsulate types and objects
// describing the statically-linked C Runtime Library
// state found in the editor.
class Crt
{
	static void* EncodePointer(void* Ptr);
public:
	static void SetNewHandler(_PNH Handler);
	static void SetPureCallHandler(_purecall_handler Handler);
	static void SetInvalidParameterHandler(_invalid_parameter_handler Handler);
};
