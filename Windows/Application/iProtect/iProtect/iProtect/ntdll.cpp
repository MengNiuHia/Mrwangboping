#include "ntdll.h"

NTSTATUS(NTAPI* LdrRegisterDllNotification)(
	_In_ ULONG Flags,
	_In_ PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
	_In_opt_ PVOID Context,
	_Out_ PVOID* Cookie
	) = nullptr;

NTSTATUS(NTAPI* LdrUnregisterDllNotification)(
	_In_ PVOID Cookie
	) = nullptr;

NTSTATUS(NTAPI* LdrFindEntryForAddress)(
	_In_ PVOID 	Address,
	_In_ PLDR_DATA_TABLE_ENTRY2* Module
	) = nullptr;

NTSTATUS(NTAPI* LdrUnloadDll)(
	_In_ HANDLE ModuleHandle
	) = nullptr;

BOOL __stdcall ResolveImports()
{
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");

	if (!ntdll)
	{
		return FALSE;
	}

	if (LdrRegisterDllNotification == nullptr)
	{
		*(void**)&LdrRegisterDllNotification = GetProcAddress(
			ntdll,
			"LdrRegisterDllNotification"
		);
	}

	if (LdrUnregisterDllNotification == nullptr)
	{
		*(void**)&LdrUnregisterDllNotification = GetProcAddress(
			ntdll,
			"LdrRegisterDllNotification"
		);
	}

	if (LdrFindEntryForAddress == nullptr)
	{
		*(void**)&LdrFindEntryForAddress = GetProcAddress(
			ntdll,
			"LdrFindEntryForAddress"
		);
	}

	if (LdrUnloadDll == nullptr)
	{
		*(void**)&LdrUnloadDll = GetProcAddress(
			ntdll,
			"LdrUnloadDll"
		);
	}

	return
		LdrRegisterDllNotification &&
		LdrUnregisterDllNotification &&
		LdrFindEntryForAddress &&
		LdrUnloadDll;
}
