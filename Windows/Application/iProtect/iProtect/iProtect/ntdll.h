#pragma once
#include <Windows.h>
#include <winternl.h>

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

#define LDR_DLL_NOTIFICATION_REASON_LOADED 1
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA
{
	ULONG Flags;                   // Reserved.
	PCUNICODE_STRING FullDllName;  // The full path name of the DLL module.
	PCUNICODE_STRING BaseDllName;  // The base file name of the DLL module.
	PVOID DllBase;                 // A pointer to the base address for the DLL in memory.
	ULONG SizeOfImage;             // The size of the DLL image, in bytes.
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA
{
	ULONG Flags;                   // Reserved.
	PCUNICODE_STRING FullDllName;  // The full path name of the DLL module.
	PCUNICODE_STRING BaseDllName;  // The base file name of the DLL module.
	PVOID DllBase;                 // A pointer to the base address for the DLL in memory.
	ULONG SizeOfImage;             // The size of the DLL image, in bytes.
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA
{
	LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
	LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;
typedef const LDR_DLL_NOTIFICATION_DATA* PCLDR_DLL_NOTIFICATION_DATA;

typedef VOID NTAPI LDR_DLL_NOTIFICATION_FUNCTION(
	_In_ ULONG NotificationReason,
	_In_ PCLDR_DLL_NOTIFICATION_DATA NotificationData,
	_In_opt_ PVOID Context
), *PLDR_DLL_NOTIFICATION_FUNCTION;

typedef struct LDR_DATA_TABLE_ENTRY2
{
	LIST_ENTRY InLoadOrderLinks; /* 0x00 */
	LIST_ENTRY InMemoryOrderLinks; /* 0x08 */
	LIST_ENTRY InInitializationOrderLinks; /* 0x10 */
	PVOID DllBase; /* 0x18 */
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName; /* 0x24 */
	UNICODE_STRING BaseDllName; /* 0x28 */
	ULONG Flags;
	WORD LoadCount;
	WORD TlsIndex;
	union
	{
		LIST_ENTRY HashLinks;
		struct
		{
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union
	{
		ULONG TimeDateStamp;
		PVOID LoadedImports;
	};
	_ACTIVATION_CONTEXT* EntryPointActivationContext;
	PVOID PatchInformation;
	LIST_ENTRY ForwarderLinks;
	LIST_ENTRY ServiceTagLinks;
	LIST_ENTRY StaticLinks;
} _LDR_DATA_TABLE_ENTRY2, *PLDR_DATA_TABLE_ENTRY2;

extern NTSTATUS(NTAPI* LdrRegisterDllNotification)(
	_In_ ULONG Flags,
	_In_ PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
	_In_opt_ PVOID Context,
	_Out_ PVOID* Cookie
	);

extern NTSTATUS(NTAPI* LdrUnregisterDllNotification)(
	_In_ PVOID Cookie
	);


extern NTSTATUS(NTAPI* LdrFindEntryForAddress)(
	_In_ PVOID 	Address,
	_In_ PLDR_DATA_TABLE_ENTRY2* Module
	);

extern NTSTATUS(NTAPI* LdrUnloadDll)(
	_In_ HANDLE ModuleHandle
	);

BOOL __stdcall ResolveImports();