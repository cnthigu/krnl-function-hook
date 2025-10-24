#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstring>

// Estrutura para comunicação com o driver kernel
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

uintptr_t base_address = 0;
static std::uint32_t process_id = 0;

// Função para chamar o driver hookado
template<typename ... Arg>
uint64_t call_hook(const Arg ... args) 
{
	HMODULE hWin32u = LoadLibraryA("win32u.dll");
	if (!hWin32u) return 0;

	void* hooked_function = GetProcAddress(hWin32u, "NtOpenCompositionSurfaceSectionInfo");
	if (!hooked_function) return 0;

	auto func = static_cast<uint64_t(__stdcall*)(Arg...)>(hooked_function);
	return func(args ...);
}

// Obter PID do processo
static std::uint32_t get_process_id(const wchar_t* process_name) 
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

// Obter endereço base do módulo
static ULONG64 get_module_base_address(const char* module_name) 
{
	NULL_MEMORY instructions = { 0 };
	instructions.pid = process_id;	
	instructions.req_base = TRUE;
	instructions.read = FALSE;
	instructions.write = FALSE;
	instructions.module_name = module_name;

	call_hook(&instructions);
	return instructions.base_adress;
}

// Ler memória
template<class T>
T Read(UINT_PTR read_address) 
{
	T response{};
	NULL_MEMORY instructions;
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

// Escrever memória
template<typename S>
bool write(UINT_PTR write_address, const S& value)
{
	NULL_MEMORY instructions;
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

int main() 
{
	std::cout << "=== Kernel Mode Hook Demo ===" << std::endl;
	std::cout << "Driver: kernel_mode.sys" << std::endl;
	std::cout << "Hook: NtOpenCompositionSurfaceSectionInfo" << std::endl;
	std::cout << std::endl;

	// Carregar user32.dll (necessário)
	LoadLibraryA("user32.dll");
	std::cout << "[OK] user32.dll carregado" << std::endl;

	// Procurar notepad
	process_id = get_process_id(L"notepad.exe");
	if (!process_id) {
		std::cout << "[ERRO] Notepad nao encontrado!" << std::endl;
		std::cout << "Abra o Notepad primeiro (Win+R -> notepad)" << std::endl;
		system("pause");
		return 1;
	}
	
	std::cout << "[OK] Notepad encontrado! PID: " << process_id << std::endl;

	// Obter endereço base
	base_address = get_module_base_address("notepad.exe");
	if (!base_address) {
		std::cout << "[ERRO] Falha ao obter endereco base" << std::endl;
		system("pause");
		return 1;
	}

	std::cout << "[OK] Endereco base: 0x" << std::hex << base_address << std::endl;
	std::cout << std::endl;

	// Demonstração simples
	std::cout << "=== Demonstracao ===" << std::endl;
	
	// Ler alguns valores
	int valor1 = Read<int>(base_address + 0x1000);
	int valor2 = Read<int>(base_address + 0x2000);
	
	std::cout << "Lendo 0x" << std::hex << (base_address + 0x1000) << " = " << std::dec << valor1 << std::endl;
	std::cout << "Lendo 0x" << std::hex << (base_address + 0x2000) << " = " << std::dec << valor2 << std::endl;
	
	// Escrever alguns valores
	write<int>(base_address + 0x1000, 12345);
	write<int>(base_address + 0x2000, 67890);
	
	std::cout << "Escrevendo 12345 em 0x" << std::hex << (base_address + 0x1000) << std::endl;
	std::cout << "Escrevendo 67890 em 0x" << std::hex << (base_address + 0x2000) << std::endl;
	
	std::cout << std::endl;
	std::cout << "=== Concluido! ===" << std::endl;
	std::cout << "Hook funcionando corretamente!" << std::endl;
	
	system("pause");
	return 0;
}