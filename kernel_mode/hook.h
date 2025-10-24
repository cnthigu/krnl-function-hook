#pragma once
#include "definitions.h"
#include "memory.h"

// ============================================================================
// NAMESPACE: nullhook
// ============================================================================
// Este namespace contém as funções principais do sistema de inline hooking
// Responsável por interceptar funções do kernel e processar comunicações
// ============================================================================
namespace nullhook
{
	// Função principal que instala o inline hook
	// Substitui os primeiros bytes da função alvo por shellcode
	bool call_kernel_function(void* kernel_function_address);
	
	// Handler que processa as requisições vindas do user mode
	// Executa operações de leitura/escrita de memória
	NTSTATUS hook_handle(PVOID called_param);
}