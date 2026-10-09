#pragma once

#include "native.h"
#include "memory.h"

bool CallKernelFunction ( void* kernel_function_address );
NTSTATUS HookHandler ( PVOID called_param );
