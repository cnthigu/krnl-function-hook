#include "hook.h"

extern "C" NTSTATUS DriverEntry ( PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path )
{
    UNREFERENCED_PARAMETER ( driver_object );
    UNREFERENCED_PARAMETER ( registry_path );

    DbgPrint ( "[Hook] Driver Load." );

    bool hook_success = CallKernelFunction ( &HookHandler );

    if ( hook_success )
    {
        DbgPrint ( "[Hook] Hook Success (NtOpenCompositionSurfaceSectionInfo)" );
    }
    else
    {
        DbgPrint ( "[Hook] Hook Failed" );
    }

    return STATUS_SUCCESS;
}
