#include "hook.h"

bool CallKernelFunction ( void* kernel_function_address )
{
    if ( !kernel_function_address )
    {
        return false;
    }

    PVOID* function = reinterpret_cast<PVOID*> ( GetSystemModuleExport ( "\\SystemRoot\\System32\\drivers\\dxgkrnl.sys",
                                                                         "NtOpenCompositionSurfaceSectionInfo" ) );

    if ( !function )
    {
        return false;
    }

    BYTE original[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    BYTE shell_code[] = { 0x48, 0xB8 };

    BYTE shell_code_end[] = { 0xFF, 0xE0 };

    RtlSecureZeroMemory ( original, sizeof ( original ) );

    memcpy ( (PVOID)( (ULONG_PTR)original ), &shell_code, sizeof ( shell_code ) );

    uintptr_t hook_address = reinterpret_cast<uintptr_t> ( kernel_function_address );

    memcpy ( (PVOID)( (ULONG_PTR)original + sizeof ( shell_code ) ), &hook_address, sizeof ( void* ) );

    memcpy ( (PVOID)( (ULONG_PTR)original + sizeof ( shell_code ) + sizeof ( void* ) ), &shell_code_end,
             sizeof ( shell_code_end ) );

    WriteToReadonlyMemory ( function, &original, sizeof ( original ) );

    return true;
}

NTSTATUS HookHandler ( PVOID called_param )
{
    if ( !called_param )
    {
        return STATUS_INVALID_PARAMETER;
    }

    HOOK_REQUEST* request = (HOOK_REQUEST*)called_param;

    if ( request->request_base == TRUE )
    {
        DbgPrint ( "[Hook] Address base: %s (PID: %d)", request->module_name, request->process_id );

        ANSI_STRING ansi_name;
        UNICODE_STRING module_name;

        RtlInitAnsiString ( &ansi_name, request->module_name );

        NTSTATUS status = RtlAnsiStringToUnicodeString ( &module_name, &ansi_name, TRUE );

        if ( !NT_SUCCESS ( status ) )
        {
            DbgPrint ( "[Hook] Error convert string: 0x%X\n", status );
            return status;
        }

        PEPROCESS process = NULL;

        status = PsLookupProcessByProcessId ( (HANDLE)request->process_id, &process );

        if ( !NT_SUCCESS ( status ) || !process )
        {
            DbgPrint ( "[Hook] Error process not found: 0x%X\n", status );
            RtlFreeUnicodeString ( &module_name );
            return status;
        }

        ULONG64 base_address64 = 0;

        base_address64 = GetModuleBaseX64 ( process, module_name );

        request->base_address = base_address64;

        DbgPrint ( "[Hook] Address base: 0x%llx", base_address64 );

        ObDereferenceObject ( process );
        RtlFreeUnicodeString ( &module_name );
        return STATUS_SUCCESS;
    }

    if ( request->write == TRUE )
    {
        DbgPrint ( "[Hook] Write in 0x%llx (size: %d)", request->address, request->size );

        if ( request->address < 0x7FFFFFFFFFFF && request->address > 0 )
        {
            PVOID kernel_buffer = ExAllocatePool ( NonPagedPool, request->size );

            if ( !kernel_buffer )
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            if ( !memcpy ( kernel_buffer, request->buffer_address, request->size ) )
            {
                return STATUS_UNSUCCESSFUL;
            }

            PEPROCESS process;

            PsLookupProcessByProcessId ( (HANDLE)request->process_id, &process );
            WriteKernelMemory ( (HANDLE)request->process_id, request->address, kernel_buffer, request->size );

            ExFreePool ( kernel_buffer );
        }
    }

    if ( request->read == TRUE )
    {
        DbgPrint ( "[Hook] Read in 0x%llx (size: %d)", request->address, request->size );

        if ( request->address < 0x7FFFFFFFFFFF && request->address > 0 )
        {
            ReadKernelMemory ( (HANDLE)request->process_id, request->address, request->output, request->size );
        }
    }

    return STATUS_SUCCESS;
}
