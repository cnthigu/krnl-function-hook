#pragma once
#include "definitions.h"
#include "memory.h"

namespace cnthook
{
	bool call_kernel_function(void* kernel_function_address);
	NTSTATUS hook_handle(PVOID called_param);
}