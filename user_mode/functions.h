#pragma once
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstring>
#include <sstream>

using namespace std;

extern uintptr_t base_address;
extern std::uint32_t process_id;

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

template<typename ... Arg>
uint64_t call_hook(const Arg ... args)
{
	HMODULE hWin32u = LoadLibraryA("win32u.dll");

	if (!hWin32u) 
		return 0;

	void* hooked_function = GetProcAddress(hWin32u, "NtOpenCompositionSurfaceSectionInfo");

	if (!hooked_function) 
		return 0;

	auto func = static_cast<uint64_t(__stdcall*)(Arg...)>(hooked_function);

	return func(args ...);
}

std::uint32_t get_process_id(const wchar_t* process_name);

ULONG64 get_module_base_address(const char* module_name);

template<class T>
T Read(UINT_PTR read_address)
{
	T response{};
	CNT_MEMORY instructions;
	instructions.pid = process_id;
	instructions.size = sizeof(T);
	instructions.address = read_address;
	instructions.read = TRUE;
	instructions.write = FALSE;
	instructions.req_base = FALSE;
	instructions.output = &response;

	call_hook(&instructions);
	return response;
}

template<typename S>
bool write(UINT_PTR write_address, const S& value)
{
	CNT_MEMORY instructions;
	instructions.address = write_address;
	instructions.pid = process_id;
	instructions.write = TRUE;
	instructions.read = FALSE;
	instructions.req_base = FALSE;
	instructions.buffer_address = (void*)&value;
	instructions.size = sizeof(S);

	call_hook(&instructions);
	return true;
}