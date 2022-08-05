#pragma once
#include <ntddk.h>

extern "C"
{
	// 拒绝加载驱动
	//void DenyLoadDriver(PVOID DriverEntry);
	NTSTATUS DenyLoadDriver(PVOID pImageBase);

	//// 拒绝加载 DLL 模块
	//BOOLEAN DenyLoadDll(PVOID pLoadImageBase);

	// 镜像加载回调函数
	VOID LoadImageNotifyRoutine(_In_ PUNICODE_STRING FullImageName, _In_ HANDLE ProcessId, _In_ PIMAGE_INFO ImageInfo);
}
