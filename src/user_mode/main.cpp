#include "hook_client.h"

#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

static ULONG GetPidByName ( const wchar_t* process_name )
{
    PROCESSENTRY32 process_entry;
    HANDLE snapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, NULL );

    if ( snapshot == INVALID_HANDLE_VALUE )
    {
        return 0;
    }

    process_entry.dwSize = sizeof ( PROCESSENTRY32 );

    if ( !Process32First ( snapshot, &process_entry ) )
    {
        CloseHandle ( snapshot );
        return 0;
    }

    while ( Process32Next ( snapshot, &process_entry ) == TRUE )
    {
        if ( wcscmp ( process_name, process_entry.szExeFile ) == 0 )
        {
            CloseHandle ( snapshot );
            return process_entry.th32ProcessID;
        }
    }

    CloseHandle ( snapshot );
    return 0;
}

int main ()
{
    LoadLibraryA ( "user32.dll" );

    ULONG process_id = GetPidByName ( L"notepad.exe" );
    if ( !process_id )
    {
        printf ( "[!] notepad.exe process not found.\n" );
        return 1;
    }

    ULONG64 base_address = GetModuleBaseDriver ( process_id, "notepad.exe" );
    if ( !base_address )
    {
        printf ( "[!] Failed to get module base.\n" );
        return 1;
    }

    UINT_PTR read_address = base_address + 0x1000;
    int value = ReadMemory<int> ( process_id, read_address );
    printf ( "Value: %d\n", value );

    UINT_PTR write_address = base_address + 0x2000;
    int write_value = 1337;
    WriteMemory<int> ( process_id, write_address, write_value );

    system ( "pause" );
    return 0;
}
