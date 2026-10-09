#include "memory.h"

PVOID GetSystemModuleBase ( const char* module_name )
{
    ULONG bytes = 0;
    NTSTATUS status = ZwQuerySystemInformation ( SystemModuleInformation, NULL, bytes, &bytes );

    if ( !bytes )
    {
        return NULL;
    }

    PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag ( NonPagedPool, bytes, 0x6e756c6c );

    status = ZwQuerySystemInformation ( SystemModuleInformation, modules, bytes, &bytes );

    if ( !NT_SUCCESS ( status ) )
    {
        return NULL;
    }

    PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;

    PVOID module_base = 0, module_size = 0;

    for ( ULONG i = 0; i < modules->NumberOfModules; i++ )
    {
        if ( _stricmp ( (char*)module[i].FullPathName, module_name ) == NULL )
        {
            module_base = module[i].ImageBase;
            module_size = (PVOID)module[i].ImageSize;
            break;
        }
    }

    if ( modules )
    {
        ExFreePoolWithTag ( modules, NULL );
    }

    if ( module_base <= NULL )
    {
        return NULL;
    }

    return module_base;
}

PVOID GetSystemModuleExport ( const char* module_name, LPCSTR routine_name )
{
    PVOID module = GetSystemModuleBase ( module_name );

    if ( module <= NULL )
    {
        return NULL;
    }

    return RtlFindExportedRoutineByName ( module, routine_name );
}

bool WriteMemory ( void* address, void* buffer, size_t size )
{
    if ( !RtlCopyMemory ( address, buffer, size ) )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool WriteToReadonlyMemory ( void* address, void* buffer, size_t size )
{
    PMDL mdl = IoAllocateMdl ( address, size, FALSE, FALSE, NULL );

    if ( !mdl )
    {
        return false;
    }

    MmProbeAndLockPages ( mdl, KernelMode, IoReadAccess );
    PVOID mapping = MmMapLockedPagesSpecifyCache ( mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority );
    MmProtectMdlSystemAddress ( mdl, PAGE_EXECUTE_READWRITE );

    WriteMemory ( mapping, buffer, size );

    MmUnmapLockedPages ( mapping, mdl );
    MmUnlockPages ( mdl );
    IoFreeMdl ( mdl );

    return true;
}

ULONG64 GetModuleBaseX64 ( PEPROCESS process, UNICODE_STRING module_name )
{
    PPEB peb = PsGetProcessPeb ( process );

    if ( !peb )
    {
        return NULL;
    }

    KAPC_STATE state;
    KeStackAttachProcess ( process, &state );

    PPEB_LDR_DATA ldr = (PPEB_LDR_DATA)peb->Ldr;

    if ( !ldr )
    {
        KeUnstackDetachProcess ( &state );
        return NULL;
    }

    for ( PLIST_ENTRY list = (PLIST_ENTRY)ldr->InMemoryOrderModuleList.Flink; list != &ldr->InMemoryOrderModuleList;
          list = (PLIST_ENTRY)list->Flink )
    {
        PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD ( list, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks );

        if ( RtlCompareUnicodeString ( &entry->BaseDllName, &module_name, TRUE ) == 0 )
        {
            ULONG64 base_address = (ULONG64)entry->DllBase;
            KeUnstackDetachProcess ( &state );
            return base_address;
        }
    }

    KeUnstackDetachProcess ( &state );
    return NULL;
}

bool ReadKernelMemory ( HANDLE process_id, UINT_PTR address, void* buffer, SIZE_T size )
{
    if ( !address || !buffer || !size )
    {
        return false;
    }

    SIZE_T bytes = 0;
    NTSTATUS status = STATUS_SUCCESS;
    PEPROCESS process;
    PsLookupProcessByProcessId ( (HANDLE)process_id, &process );

    status = MmCopyVirtualMemory ( process, (void*)address, (PEPROCESS)PsGetCurrentProcess (), (void*)buffer, size,
                                   KernelMode, &bytes );

    if ( !NT_SUCCESS ( status ) )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool WriteKernelMemory ( HANDLE process_id, uintptr_t address, void* buffer, SIZE_T size )
{
    if ( !address || !buffer || !size )
    {
        return false;
    }

    SIZE_T bytes = 0;
    NTSTATUS status = STATUS_SUCCESS;
    PEPROCESS process;
    PsLookupProcessByProcessId ( (HANDLE)process_id, &process );

    KAPC_STATE state;

    KeStackAttachProcess ( (PEPROCESS)process, &state );

    MEMORY_BASIC_INFORMATION info;

    status = ZwQueryVirtualMemory ( ZwCurrentProcess (), (PVOID)address, MemoryBasicInformation, &info, sizeof ( info ),
                                    NULL );

    if ( !NT_SUCCESS ( status ) )
    {
        KeUnstackDetachProcess ( &state );
        return false;
    }

    if ( ( (uintptr_t)info.BaseAddress + info.RegionSize ) < ( address + size ) )
    {
        KeUnstackDetachProcess ( &state );
        return false;
    }

    if ( !( info.State & MEM_COMMIT ) || ( info.Protect & ( PAGE_GUARD | PAGE_NOACCESS ) ) )
    {
        KeUnstackDetachProcess ( &state );
        return false;
    }

    if ( ( info.Protect & PAGE_EXECUTE_READWRITE ) || ( info.Protect & PAGE_EXECUTE_WRITECOPY ) ||
         ( info.Protect & PAGE_READWRITE ) || ( info.Protect & PAGE_WRITECOPY ) )
    {
        RtlCopyMemory ( (void*)address, buffer, size );
    }

    KeUnstackDetachProcess ( &state );
    return false;
}
