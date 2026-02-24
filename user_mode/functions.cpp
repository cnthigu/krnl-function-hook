#include "functions.h"

uintptr_t base_address = 0;
std::uint32_t process_id = 0;

std::uint32_t get_process_id(const wchar_t* process_name)
{
	PROCESSENTRY32 processentry;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (snapshot == INVALID_HANDLE_VALUE) return 0;

	processentry.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(snapshot, &processentry)) {
		CloseHandle(snapshot);
		return 0;
	}

	while (Process32Next(snapshot, &processentry) == TRUE) {
		if (wcscmp(process_name, processentry.szExeFile) == 0) {
			CloseHandle(snapshot);
			return processentry.th32ProcessID;
		}
	}

	CloseHandle(snapshot);
	return 0;
}

ULONG64 get_module_base_address(const char* module_name)
{
	CNT_MEMORY instructions = { 0 };
	instructions.pid = process_id;
	instructions.req_base = TRUE;
	instructions.read = FALSE;
	instructions.write = FALSE;
	instructions.module_name = module_name;

	call_hook(&instructions);
	return instructions.base_adress;
}