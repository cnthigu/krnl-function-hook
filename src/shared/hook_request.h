#pragma once

#ifdef _KERNEL_MODE
#include <ntifs.h>
#else
#include <windows.h>
#endif

typedef struct _HOOK_REQUEST
{
    void* buffer_address;
    UINT_PTR address;
    ULONGLONG size;
    ULONG process_id;
    BOOLEAN write;
    BOOLEAN read;
    BOOLEAN request_base;
    void* output;
    const char* module_name;
    ULONG64 base_address;
} HOOK_REQUEST, *PHOOK_REQUEST;
