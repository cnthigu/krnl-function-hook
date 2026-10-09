#pragma once

#include <ntdef.h>
#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <ntstrsafe.h>
#include <wdm.h>
#pragma comment( lib, "ntoskrnl.lib" )

#ifndef _SYSTEM_INFOMATION_CLASS_DEFINED
#define _SYSTEM_INFOMATION_CLASS_DEFINED
typedef enum _SYSTEM_INFOMATION_CLASS
{
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPatchInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation = 0x0B
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;
#endif

#ifndef _RTL_PROCESS_MODULE_INFORMATION_DEFINED
#define _RTL_PROCESS_MODULE_INFORMATION_DEFINED
typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;
#endif

#ifndef _RTL_PROCESS_MODULES_DEFINED
#define _RTL_PROCESS_MODULES_DEFINED
typedef struct _RTL_PROCESS_MODULES
{
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;
#endif

extern "C" __declspec ( dllimport ) NTSTATUS NTAPI ZwProtectVirtualMemomry ( HANDLE process_handle, PVOID* base_address,
                                                                             PULONG protect_size, ULONG new_protection,
                                                                             PULONG old_protection );

extern "C" NTKERNELAPI PVOID NTAPI RtlFindExportedRoutineByName ( _In_ PVOID image_base, _In_ PCCH routine_name );

extern "C" NTSTATUS ZwQuerySystemInformation ( ULONG info_class, PVOID buffer, ULONG length, PULONG return_length );

extern "C" NTKERNELAPI PPEB PsGetProcessPeb ( _In_ PEPROCESS process );

extern "C" NTSTATUS NTAPI MmCopyVirtualMemory ( PEPROCESS source_process, PVOID source_address,
                                                PEPROCESS target_process, PVOID target_address, SIZE_T buffer_size,
                                                KPROCESSOR_MODE previous_mode, PSIZE_T return_size );

extern "C" NTSTATUS NTAPI ZwQueryVirtualMemory ( HANDLE process_handle, PVOID base_address,
                                                 MEMORY_INFORMATION_CLASS memory_information_class,
                                                 PVOID memory_information, SIZE_T memory_information_length,
                                                 PSIZE_T return_length );

typedef VOID ( *PPS_POST_PROCESS_INIT_ROUTINE ) ( VOID );

#ifndef _RTL_USER_PROCESS_PARAMETERS_DEFINED
#define _RTL_USER_PROCESS_PARAMETERS_DEFINED
typedef struct _RTL_USER_PROCESS_PARAMETERS
{
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;
#endif

#ifndef _PEB_LDR_DATA_DEFINED
#define _PEB_LDR_DATA_DEFINED
typedef struct _PEB_LDR_DATA
{
    BYTE Reserved1[8];
    PVOID Reserved2[3];
    LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;
#endif

#ifndef _PEB_DEFINED
#define _PEB_DEFINED
typedef struct _PEB
{
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    BYTE Reserved4[104];
    PVOID Reserved5[52];
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    BYTE Reserved6[128];
    PVOID Reserved7[1];
    ULONG SessionId;
} PEB, *PPEB;
#endif

#ifndef _LDR_DATA_TABLE_ENTRY_DEFINED
#define _LDR_DATA_TABLE_ENTRY_DEFINED
typedef struct _LDR_DATA_TABLE_ENTRY
{
    PVOID Reserved1[2];
    LIST_ENTRY InMemoryOrderLinks;
    PVOID Reserved2[2];
    PVOID DllBase;
    PVOID Reserved3[2];
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    BYTE Reserved4[8];
    PVOID Reserved5[3];
    union
    {
        ULONG CheckSum;
        PVOID Reserved6;
    };
    ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
#endif
