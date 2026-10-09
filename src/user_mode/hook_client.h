#pragma once

#include <cstdint>
#include <windows.h>

#include "../shared/hook_request.h"

template <typename... Arg> uint64_t CallHook ( const Arg... args )
{
    HMODULE win32u = LoadLibraryA ( "win32u.dll" );

    if ( !win32u )
    {
        return 0;
    }

    void* hooked_function = GetProcAddress ( win32u, "NtOpenCompositionSurfaceSectionInfo" );

    if ( !hooked_function )
    {
        return 0;
    }

    auto function = static_cast<uint64_t ( __stdcall* ) ( Arg... )> ( hooked_function );

    return function ( args... );
}

ULONG64 GetModuleBaseDriver ( ULONG process_id, const char* module_name );

template <class T> T ReadMemory ( ULONG process_id, UINT_PTR address )
{
    T response{};
    HOOK_REQUEST request;
    request.process_id = process_id;
    request.size = sizeof ( T );
    request.address = address;
    request.read = TRUE;
    request.write = FALSE;
    request.request_base = FALSE;
    request.output = &response;

    CallHook ( &request );
    return response;
}

template <class T> bool WriteMemory ( ULONG process_id, UINT_PTR address, const T& value )
{
    HOOK_REQUEST request;
    request.address = address;
    request.process_id = process_id;
    request.write = TRUE;
    request.read = FALSE;
    request.request_base = FALSE;
    request.buffer_address = (void*)&value;
    request.size = sizeof ( T );

    CallHook ( &request );
    return true;
}
