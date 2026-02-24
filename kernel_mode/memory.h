#pragma once
#include "definitions.h"

typedef struct _CNT_MEMORY
{
	void* buffer_address;
	UINT_PTR address;
	ULONGLONG size;
	ULONG pid;
	BOOLEAN write;
	BOOLEAN read;
	BOOLEAN req_base;
	void* output;
	const char* module_name;
	ULONG64 base_adress;
}CNT_MEMORY;

PVOID get_system_module_base(const char* module_name);

PVOID get_system_module_export(const char* module_name, LPCSTR routime_name);

bool write_memory(void* address, void* buffer, size_t size);

bool write_to_readonly_memory(void* address, void* buffer, size_t size);

ULONG64 get_module_base_x64(PEPROCESS proc, UNICODE_STRING module_name);

bool read_kernel_memory(HANDLE pid, UINT_PTR address, void* buffer, SIZE_T size);

bool write_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size);
