#pragma once

#include "native.h"
#include "../shared/hook_request.h"

PVOID GetSystemModuleBase ( const char* module_name );
PVOID GetSystemModuleExport ( const char* module_name, LPCSTR routine_name );
bool WriteMemory ( void* address, void* buffer, size_t size );
bool WriteToReadonlyMemory ( void* address, void* buffer, size_t size );
ULONG64 GetModuleBaseX64 ( PEPROCESS process, UNICODE_STRING module_name );
bool ReadKernelMemory ( HANDLE process_id, UINT_PTR address, void* buffer, SIZE_T size );
bool WriteKernelMemory ( HANDLE process_id, uintptr_t address, void* buffer, SIZE_T size );
