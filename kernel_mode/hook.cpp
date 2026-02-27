#include "hook.h"

bool cnthook::call_kernel_function(void* kernel_function_address) 
{
	if (!kernel_function_address)
		return false;

	PVOID* function = reinterpret_cast<PVOID*>(get_system_module_export("\\SystemRoot\\System32\\drivers\\dxgkrnl.sys", 
																		"NtOpenCompositionSurfaceSectionInfo"));
	if (!function)
		return false;

	BYTE orig[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	BYTE shell_code[] = { 0x48, 0xB8 };
	
	BYTE shell_code_end[] = { 0xFF, 0xE0 };

	RtlSecureZeroMemory(orig, sizeof(orig));
	
	memcpy((PVOID)((ULONG_PTR)orig), &shell_code, sizeof(shell_code));
	
	uintptr_t hook_address = reinterpret_cast<uintptr_t>(kernel_function_address);

	memcpy((PVOID)((ULONG_PTR)orig + sizeof(shell_code)), &hook_address, sizeof(void*));
	
	memcpy((PVOID)((ULONG_PTR)orig + sizeof(shell_code) + sizeof(void*)), &shell_code_end, sizeof(shell_code_end));

	write_to_readonly_memory(function, &orig, sizeof(orig));

	return true;
}

NTSTATUS cnthook::hook_handle(PVOID called_param) 
{
	if (!called_param)
		return STATUS_INVALID_PARAMETER;

	CNT_MEMORY* instructions = (CNT_MEMORY*)called_param;

	if (instructions->req_base == TRUE)
	{
		DbgPrint("[Hook] Address base: %s (PID: %d)", instructions->module_name, instructions->pid);
		
		ANSI_STRING AS;
		UNICODE_STRING ModuleName;
		
		RtlInitAnsiString(&AS, instructions->module_name);
		
		NTSTATUS status = RtlAnsiStringToUnicodeString(&ModuleName, &AS, TRUE);

		if (!NT_SUCCESS(status))
		{
			DbgPrint("[Hook] Error convert string: 0x%X\n", status);
			return status;
		}
			

		PEPROCESS process = NULL;

		status = PsLookupProcessByProcessId((HANDLE)instructions->pid, &process);
		
		if (!NT_SUCCESS(status) || !process)
		{
			DbgPrint("[Hook] Error process not found: 0x%X\n", status);
			RtlFreeUnicodeString(&ModuleName);
			return status;
		}


		ULONG64 base_address64 = 0;	

		base_address64 = get_module_base_x64(process, ModuleName);

		instructions->base_adress = base_address64;
		
		DbgPrint("[Hook] Address base: 0x%llx", base_address64);
		

		ObDereferenceObject(process);
		RtlFreeUnicodeString(&ModuleName);
		return STATUS_SUCCESS;
	}	


	if (instructions->write == TRUE) 
	{
		DbgPrint("[Hook] Write in 0x%llx (size: %d)", instructions->address, instructions->size);
		
		if (instructions->address < 0x7FFFFFFFFFFF && instructions->address > 0)
		{
			PVOID kernelBuff = ExAllocatePool(NonPagedPool, instructions->size);

			if (!kernelBuff) 
			{
				return STATUS_INSUFFICIENT_RESOURCES;
			}

			if (!memcpy(kernelBuff, instructions->buffer_address, instructions->size))
			{
				return STATUS_UNSUCCESSFUL;
			}

			PEPROCESS process;
			
			PsLookupProcessByProcessId((HANDLE)instructions->pid, &process);
			write_kernel_memory((HANDLE)instructions->pid, instructions->address, kernelBuff, instructions->size);
			
			// Liberar buffer do kernel
			ExFreePool(kernelBuff);
		}
	}

	if(instructions->read == TRUE) 
	{
		DbgPrint("[Hook] Read in 0x%llx (size: %d)", instructions->address, instructions->size);
		
		if (instructions->address < 0x7FFFFFFFFFFF && instructions->address > 0)
		{
			read_kernel_memory((HANDLE)instructions->pid, instructions->address, instructions->output, instructions->size);
		}
	}
	
	return STATUS_SUCCESS;
}