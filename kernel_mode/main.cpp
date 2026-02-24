#include "hook.h"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING reg_path) 
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(reg_path);

	DbgPrint("[Hook] Driver Load.");

	bool hook_success = cnthook::call_kernel_function(&cnthook::hook_handle);
	
	if (hook_success) 
	{
		DbgPrint("[Hook] Hook Sucess (NtOpenCompositionSurfaceSectionInfo)");
	} 
	else 
	{
		DbgPrint("[Hook] Hook Failed");
	}

	return STATUS_SUCCESS;
}	