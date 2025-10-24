#include "hook.h"

// ============================================================================
// FUNÇÃO: call_kernel_function
// ============================================================================
// Esta é a função principal que instala o inline hook
// Ela substitui os primeiros bytes da função NtOpenCompositionSurfaceSectionInfo
// por um shellcode que redireciona a execução para nossa função hook_handle
// ============================================================================
bool nullhook::call_kernel_function(void* kernel_function_address) 
{
	if (!kernel_function_address)
		return false;

	// IMPORTANTE: Esta é a mesma função que o user mode chama!
	// O driver dxgkrnl.sys contém a função que vamos interceptar
	PVOID* function = reinterpret_cast<PVOID*>(get_system_module_export("\\SystemRoot\\System32\\drivers\\dxgkrnl.sys", 
																		"NtOpenCompositionSurfaceSectionInfo"));
	if (!function)
		return false;

	// Buffer para armazenar os bytes originais da função
	BYTE orig[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// SHELLCODE: MOV RAX, <endereço>
	// 0x48 = REX.W prefix (64-bit)
	// 0xB8 = MOV RAX, immediate
	BYTE shell_code[] = { 0x48, 0xB8 };
	
	// SHELLCODE: JMP RAX
	// 0xFF = ModR/M byte
	// 0xE0 = JMP RAX instruction
	BYTE shell_code_end[] = { 0xFF, 0xE0 };

	// Limpar o buffer original
	RtlSecureZeroMemory(orig, sizeof(orig));
	
	// Construir o shellcode completo:
	// 1. Copiar MOV RAX instruction
	memcpy((PVOID)((ULONG_PTR)orig), &shell_code, sizeof(shell_code));
	
	// 2. Copiar o endereço da nossa função (8 bytes)
	uintptr_t hook_address = reinterpret_cast<uintptr_t>(kernel_function_address);
	memcpy((PVOID)((ULONG_PTR)orig + sizeof(shell_code)), &hook_address, sizeof(void*));
	
	// 3. Copiar JMP RAX instruction
	memcpy((PVOID)((ULONG_PTR)orig + sizeof(shell_code) + sizeof(void*)), &shell_code_end, sizeof(shell_code_end));

	// Escrever o shellcode na função original usando MDL
	write_to_readonly_memory(function, &orig, sizeof(orig));

	return true;
}

// ============================================================================
// FUNÇÃO: hook_handle
// ============================================================================
// Esta é a função que será executada quando a função hookada for chamada
// Ela processa as requisições do user mode e executa operações de memória
// ============================================================================
NTSTATUS nullhook::hook_handle(PVOID called_param) 
{
	// Validar ponteiro de entrada
	if (!called_param)
		return STATUS_INVALID_PARAMETER;

	// Cast para nossa estrutura de comunicação
	NULL_MEMORY* instructions = (NULL_MEMORY*)called_param;

	// ============================================
	// OPERAÇÃO: SOLICITAR ENDEREÇO BASE DE MÓDULO
	// ============================================
	if (instructions->req_base == TRUE)
	{
		DbgPrint("[KERNEL] Solicitando endereco base: %s (PID: %d)", 
				 instructions->module_name, instructions->pid);
		
		// Converter string ANSI para UNICODE
		ANSI_STRING AS;
		UNICODE_STRING ModuleName;
		
		RtlInitAnsiString(&AS, instructions->module_name);
		
		NTSTATUS status = RtlAnsiStringToUnicodeString(&ModuleName, &AS, TRUE);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("[HOOK] Falha ao converter string: 0x%X\n", status);
			return status;
		}

		// Obter PEPROCESS do processo alvo
		PEPROCESS process = NULL;
		status = PsLookupProcessByProcessId((HANDLE)instructions->pid, &process);
		
		if (!NT_SUCCESS(status) || !process)
		{
			DbgPrint("[HOOK] Falha ao encontrar processo: 0x%X\n", status);
			RtlFreeUnicodeString(&ModuleName);
			return status;
		}

		// Obter endereço base real do módulo
		ULONG64 base_address64 = 0;	
		base_address64 = get_module_base_x64(process, ModuleName);
		instructions->base_adress = base_address64;
		
		DbgPrint("[KERNEL] Endereco base encontrado: 0x%llx", base_address64);
		
		// Limpeza de recursos
		ObDereferenceObject(process);
		RtlFreeUnicodeString(&ModuleName);
		return STATUS_SUCCESS;
	}	

	// ============================================
	// OPERAÇÃO: ESCRITA DE MEMÓRIA
	// ============================================
	if (instructions->write == TRUE) 
	{
		DbgPrint("[KERNEL] Operacao de ESCRITA em 0x%llx (tamanho: %d)", 
				 instructions->address, instructions->size);
		
		// Validar endereço (espaço de usuário)
		if (instructions->address < 0x7FFFFFFFFFFF && instructions->address > 0)
		{
			// Alocar buffer no kernel para os dados
			PVOID kernelBuff = ExAllocatePool(NonPagedPool, instructions->size);

			if (!kernelBuff) 
			{
				return STATUS_INSUFFICIENT_RESOURCES;
			}

			// Copiar dados do user mode para o kernel
			if (!memcpy(kernelBuff, instructions->buffer_address, instructions->size))
			{
				return STATUS_UNSUCCESSFUL;
			}

			// Obter PEPROCESS e escrever na memória do processo alvo
			PEPROCESS process;
			
			PsLookupProcessByProcessId((HANDLE)instructions->pid, &process);
			write_kernel_memory((HANDLE)instructions->pid, instructions->address, kernelBuff, instructions->size);
			
			// Liberar buffer do kernel
			ExFreePool(kernelBuff);
		}
	}

	// ============================================
	// OPERAÇÃO: LEITURA DE MEMÓRIA
	// ============================================
	if(instructions->read == TRUE) 
	{
		DbgPrint("[KERNEL] Operacao de LEITURA em 0x%llx (tamanho: %d)", 
				 instructions->address, instructions->size);
		
		// Validar endereço (espaço de usuário)
		if (instructions->address < 0x7FFFFFFFFFFF && instructions->address > 0)
		{
			// Ler memória do processo alvo para o buffer de saída
			read_kernel_memory((HANDLE)instructions->pid, instructions->address, instructions->output, instructions->size);
		}
	}
	
	return STATUS_SUCCESS;
}