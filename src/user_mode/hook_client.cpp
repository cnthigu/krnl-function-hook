#include "hook_client.h"

ULONG64 GetModuleBaseDriver ( ULONG process_id, const char* module_name )
{
    HOOK_REQUEST request = { 0 };
    request.process_id = process_id;
    request.request_base = TRUE;
    request.read = FALSE;
    request.write = FALSE;
    request.module_name = module_name;

    CallHook ( &request );
    return request.base_address;
}
