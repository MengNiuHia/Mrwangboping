#pragma once
#include <atlacc.h>
#include "ntdll.h"

typedef BOOL(WINAPI* LdrDllHandler)(
	_In_ PLDR_DATA_TABLE_ENTRY2 Module,
	_In_ BOOL bIsLoading
	);

class LdrInstance
{
private:
	CSimpleArray<LdrDllHandler> CallbackList;

	friend VOID NTAPI glNotificationTrigger(
		_In_ ULONG NotificationReason,
		_In_ PCLDR_DLL_NOTIFICATION_DATA NotificationData,
		_In_opt_ PVOID Context
	);

	__inline VOID __stdcall callAllContexts(
		_In_ PLDR_DATA_TABLE_ENTRY2 Module,
		_In_ BOOL bIsLoading
	);
public:
	LdrInstance();

	~LdrInstance();

	BOOL __stdcall RegisterWatchContext(
		_In_ LdrDllHandler Context
	);

	BOOL __stdcall UnregisterWatchContext(
		_In_ LdrDllHandler Context
	);

	BOOL __stdcall Initialize();

	VOID __stdcall Destroy();
};