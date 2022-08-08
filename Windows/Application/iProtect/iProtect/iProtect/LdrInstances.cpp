#include "LdrInstances.h"

class LdrInstance;

PVOID registerCookie = 0;

CSimpleArray<LdrInstance*>* glGetLdrInstances()
{
	static CSimpleArray<LdrInstance*>* glRootLdrInstances = nullptr;

	if (glRootLdrInstances == nullptr)
	{
		glRootLdrInstances = new CSimpleArray<LdrInstance*>();
	}

	return glRootLdrInstances;
}

VOID NTAPI glNotificationTrigger(
	_In_ ULONG NotificationReason,
	_In_ PCLDR_DLL_NOTIFICATION_DATA NotificationData,
	_In_opt_ PVOID Context
)
{
	CSimpleArray<LdrInstance*>* instances = glGetLdrInstances();
	INT countInstances = 0;
	INT countContextHandler = 0;

	INT i = 0;

	PLDR_DATA_TABLE_ENTRY2 onListEntry = 0;
	BOOL isLoading = (NotificationReason == LDR_DLL_NOTIFICATION_REASON_LOADED);

	LdrFindEntryForAddress(
		isLoading ? NotificationData->Loaded.DllBase :
		NotificationData->Unloaded.DllBase,
		&onListEntry
	);

	if (instances == nullptr || onListEntry == nullptr)
	{
		return;
	}

	countInstances = instances->GetSize();

	for (i = 0; i < countInstances; ++i)
	{
		(*instances)[i]->callAllContexts(onListEntry, isLoading);
	}

}

BOOL __stdcall glStartNotify()
{
	return LdrRegisterDllNotification(
		0,
		glNotificationTrigger,
		0,
		&registerCookie
	) == STATUS_SUCCESS;
}

BOOL __stdcall glStopNotify()
{
	return LdrUnregisterDllNotification(
		registerCookie
	) == STATUS_SUCCESS;
}

__inline VOID __stdcall LdrInstance::callAllContexts(
	_In_ PLDR_DATA_TABLE_ENTRY2 Module,
	_In_ BOOL bIsLoading
)
{
	INT countContexts = 0;
	INT i = 0;

	countContexts = CallbackList.GetSize();

	for (i = 0; i < countContexts; ++i)
	{
		if (!CallbackList[i](Module, bIsLoading))
		{
			return;
		}
	}

	return;
}

LdrInstance::LdrInstance()
{

}

LdrInstance::~LdrInstance()
{

}

BOOL __stdcall LdrInstance::RegisterWatchContext(
	_In_ LdrDllHandler Context
)
{
	INT onListIndex = 0;

	if (Context == NULL)
	{
		return FALSE;
	}

	onListIndex = CallbackList.Find(Context);

	if (onListIndex == -1)
	{
		return CallbackList.Add(Context);
	}

	return FALSE;
}

BOOL __stdcall LdrInstance::UnregisterWatchContext(
	_In_ LdrDllHandler Context
)
{
	INT onListIndex = 0;

	if (Context == NULL)
	{
		return FALSE;
	}

	onListIndex = CallbackList.Find(Context);

	if (onListIndex >= 0)
	{
		return CallbackList.RemoveAt(onListIndex);
	}

	return FALSE;
}

BOOL __stdcall LdrInstance::Initialize()
{
	if (!ResolveImports())
	{
		return FALSE;
	}

	if (::glGetLdrInstances()->GetSize() == 0)
	{
		if (!::glStartNotify())
		{
			return FALSE;
		}

		::glGetLdrInstances()->Add(this);
	}

	return TRUE;
}

VOID __stdcall LdrInstance::Destroy()
{
	CallbackList.RemoveAll();

	::glGetLdrInstances()->Remove(this);

	if (::glGetLdrInstances()->GetSize() == 0)
	{
		::glStopNotify();
	}
}
