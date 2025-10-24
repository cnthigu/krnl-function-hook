#include "hook.h"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING reg_path) 
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(reg_path);

	DbgPrint("[KERNEL] Driver carregado com sucesso!");
	DbgPrint("[KERNEL] Iniciando instalacao do hook...");

	bool hook_success = nullhook::call_kernel_function(&nullhook::hook_handle);
	
	if (hook_success) {
		DbgPrint("[KERNEL] Hook instalado com sucesso!");
		DbgPrint("[KERNEL] Interceptando: NtOpenCompositionSurfaceSectionInfo");
	} else {
		DbgPrint("[KERNEL] ERRO: Falha ao instalar hook!");
	}

	return STATUS_SUCCESS;
}	