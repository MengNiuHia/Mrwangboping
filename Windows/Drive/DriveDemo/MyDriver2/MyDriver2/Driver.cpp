#include "Driver.hpp"


HANDLE g_ProcessId = NULL;


//直接返回完成信息	
NTSTATUS DispatchCreateClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//完成此请求
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//I/O派遣例程处理
NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("monitorProc : DispatchIotcl...\n");
	//默认返回失败
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	//取得此IRP的I/O堆栈指针
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	//取得设备扩展结构指针
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	//获取I/O控制代码
	ULONG uIoContrlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	//取得I/O缓冲区的指针和其长度
	pCallbackInfo pCallbackInfoBuff = (pCallbackInfo)pIrp->AssociatedIrp.SystemBuffer;
	ULONG uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;//输入缓冲区长度
	ULONG uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;//输出缓冲区长度
	switch (uIoContrlCode)
	{
	case IOCTL_NTPROCDRV_GET_PROCINFO:   //想用户程序返回有事件发生的进程信息
	{
		if (uOutSize >= sizeof(CallbackInfo)) //输出缓冲区长度要大于返回结构大小
		{
			pCallbackInfoBuff->hParentId = pDevExt->hPParentId;
			pCallbackInfoBuff->hProcessId = pDevExt->hPProcessId;
			pCallbackInfoBuff->bCreate = pDevExt->bPCreate;
			status = STATUS_SUCCESS;
		}
		DbgPrint("dispatch pCallbackInfoBuff->hProcessId = %d\n", pCallbackInfoBuff->hProcessId);

	}
	break;
	}
	if (status == STATUS_SUCCESS)
		pIrp->IoStatus.Information = uOutSize;
	else
		pIrp->IoStatus.Information = 0;
	//完成请求
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
//进程监视回调函数
VOID ProcessMonitorCallback(
	IN HANDLE hParentId,
	IN HANDLE hProcessId,
	IN BOOLEAN bCreate)
{
	//得到设备扩展的结构指针
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)g_pDeviceObject->DeviceExtension;
	//先保存在扩展处理当前的设备扩展结构，传给应用层
	pDevExt->hPParentId = hParentId;
	pDevExt->hPProcessId = hProcessId;
	pDevExt->bPCreate = bCreate;
	DbgPrint("hProcessId = %d\n", hProcessId);
	//触发，通知应用层监听程序，第三个参数是 是否等待，如果TRUE,需要驱动调用kewait
	// 如果事件是通知事件，则系统会尝试满足事件对象上尽可能多的等待。 在对 KeClearEvent 或 KeResetEvent 的调用清除之前，事件将保持信号。 如果事件是同步事件，则系统自动清除事件之前，会先等待一次。
	KeSetEvent(pDevExt->pEvent, 0, FALSE);
	//设置为非授信状态
	KeClearEvent(pDevExt->pEvent);

}

// 拒绝加载驱动
NTSTATUS DenyLoadDriver(PVOID pImageBase)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMDL pMdl = NULL;
	PVOID pVoid = NULL;
	ULONG ulShellcodeLength = 16;
	UCHAR pShellcode[16] = { 0xB8, 0x22, 0x00, 0x00, 0xC0, 0xC3, 0x90, 0x90,
							0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pImageBase;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
	//找到DriverEntry的基址
	PVOID pDriverEntry = (PVOID)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.AddressOfEntryPoint);

	pMdl = MmCreateMdl(NULL, pDriverEntry, ulShellcodeLength);
	MmBuildMdlForNonPagedPool(pMdl);
	pVoid = MmMapLockedPages(pMdl, KernelMode);
	RtlCopyMemory(pVoid, pShellcode, ulShellcodeLength);
	MmUnmapLockedPages(pVoid, pMdl);
	IoFreeMdl(pMdl);

	return status;
}


// 拒绝加载 DLL 模块
BOOLEAN DenyLoadDll(PVOID pLoadImageBase)
{
	// DLL拒绝加载, 不能类似驱动那样直接在入口点返回拒绝加载信息. 这样达不到卸载DLL的效果.
	// 将文件头 前0x200 字节数据置零

	ULONG ulDataSize = 0x200;
	// 创建 MDL 方式修改内存
	PMDL pMdl = MmCreateMdl(NULL, pLoadImageBase, ulDataSize);
	if (NULL == pMdl)
	{
		DbgPrint("MmCreateMdl");
		return FALSE;
	}
	MmBuildMdlForNonPagedPool(pMdl);
	PVOID pVoid = MmMapLockedPages(pMdl, KernelMode);
	if (NULL == pVoid)
	{
		IoFreeMdl(pMdl);
		DbgPrint("MmMapLockedPages");
		return FALSE;
	}
	// 置零
	RtlZeroMemory(pVoid, ulDataSize);
	// 释放 MDL
	MmUnmapLockedPages(pVoid, pMdl);
	IoFreeMdl(pMdl);

	return TRUE;
}

// 回调函数
VOID LoadImageNotifyRoutine(_In_ PUNICODE_STRING FullImageName, _In_ HANDLE ProcessId, _In_ PIMAGE_INFO ImageInfo)
{
	// 显示加载模块信息
	DbgPrint("[ProcessId = %d][FullImageName = %wZ][ImageSize = %d][ImageBase = 0x%p]\n", ProcessId, FullImageName, ImageInfo->ImageSize, ImageInfo->ImageBase);

	// 拒绝加载指定模块
	if (NULL != wcsstr(FullImageName->Buffer, L"Driver3.sys") ||
		NULL != wcsstr(FullImageName->Buffer, L"Test.dll"))
	{
		// Driver
		if (0 == ProcessId)
		{
			DbgPrint("Deny Load Driver\n");
			DenyLoadDriver(ImageInfo->ImageBase);
		}
		// Dll
		else
		{
			DbgPrint("Deny Load DLL\n");
			DenyLoadDll(ImageInfo->ImageBase);
		}
	}

	//拒绝Everything.exe加载dll
	if (NULL != wcsstr(FullImageName->Buffer, L"Everything.exe"))
	{
		DbgPrint("发现Everything进程 进程id = %d\n", ProcessId);
		//保存一下进程ID
		g_ProcessId = ProcessId;
	}

	//禁止该进程ID加载dll
	if (g_ProcessId == ProcessId && NULL != wcsstr(FullImageName->Buffer, L".dll"))
	{
		DbgPrint("Deny Load DLL\n");
		DenyLoadDll(ImageInfo->ImageBase);
	}

}

VOID DrvUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status;
	status  = PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsRemoveLoadImageNotifyRoutine status = %d", status);
	}

	////移除通知函数
	//PsSetCreateProcessNotifyRoutine(ProcessMonitorCallback, TRUE);

	//关闭event
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)g_pDeviceObject->DeviceExtension;
	KeClearEvent(pDevExt->pEvent);
	ZwClose(pDevExt->hEventHandle);
	//删除设备连接符号
	UNICODE_STRING strLink;
	RtlInitUnicodeString(&strLink, LINK_NAME);
	IoDeleteSymbolicLink(&strLink);
	IoDeleteDevice(pDriver->DeviceObject); //删除设备对象

	DbgPrint("驱动卸载成功！\n");
	KdPrint(("驱动卸载成功\n"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING reg_path)
{
	NTSTATUS status = STATUS_SUCCESS;
	//初始化各个历程
	pDriver->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;
	pDriver->DriverUnload = DrvUnload;


	//创建，并初始化设备对象
	UNICODE_STRING strDevName;
	RtlInitUnicodeString(&strDevName, DEVICE_NAME);
	//创建设备对象
	PDEVICE_OBJECT pDevObj;
	status = IoCreateDevice(
		pDriver,
		sizeof(DEVICE_EXTENSION),//为设备扩展结构申请空间
		&strDevName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&pDevObj
	);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	//创建符号链接
	UNICODE_STRING strLinkName;
	RtlInitUnicodeString(&strLinkName, LINK_NAME);
	//创建 关联
	status = IoCreateSymbolicLink(&strLinkName, &strDevName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	//将设备指针保存到全局变量中，方便后续使用
	g_pDeviceObject = pDevObj;
	//为了能够监视用户层进程，创建事件对象
	UNICODE_STRING szpEventString;
	RtlInitUnicodeString(&szpEventString, EVENT_NAME);
	//创建一个event对象
	pDevExt->pEvent = IoCreateSynchronizationEvent(&szpEventString, &pDevExt->hEventHandle);
	//设置非授信状态
	KeClearEvent(pDevExt->pEvent);

	status = PsSetLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsSetLoadImageNotifyRoutine status = %d", status);
	}

	////设置回调函数历程
	//status = PsSetCreateProcessNotifyRoutine(ProcessMonitorCallback, FALSE);

	DbgPrint("驱动加载成功！\n");
	KdPrint(("驱动加载成功\n"));
	return status;
}
