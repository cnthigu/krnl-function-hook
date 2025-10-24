#pragma once
#include "definitions.h"

typedef struct _NULL_MEMORY
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
}NULL_MEMORY;


// Encontra o endereço base de um módulo (driver) do sistema
PVOID get_system_module_base(const char* module_name);

// Encontra uma função exportada dentro de um módulo
PVOID get_system_module_export(const char* module_name, LPCSTR routime_name);

// Escreve dados em um endereço de memória
bool write_memory(void* address, void* buffer, size_t size);

// Escreve dados em memória protegida (read-only) usando MDL
bool write_to_readonly_memory(void* address, void* buffer, size_t size);

// Pega o endereço base de um módulo (DLL) carregado em um processo usermode
ULONG64 get_module_base_x64(PEPROCESS proc, UNICODE_STRING module_name);

// Lê memória de um processo
bool read_kernel_memory(HANDLE pid, UINT_PTR address, void* buffer, SIZE_T size);

// Escreve memória em um processo
bool write_kernel_memory(HANDLE pid, uintptr_t address, void* buffer, SIZE_T size);