#include <ntifs.h>
#include <ntimage.h>
#include <devioctl.h>

#include "Driver.hpp"
#include "ProcessWatch.hpp"
#include "WFPNetworkWatch.hpp"
#include "LoadImageWatch.hpp"

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

VOID DrvUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status;

	////移除网络监控通知函数
	//status =  WallUnRegisterCallouts();

	//移除镜像加载通知函数
	status  = PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);

	////移除进程创建通知函数
	//status = PsSetCreateProcessNotifyRoutine(ProcessMonitorCallback, TRUE);
	//if (!NT_SUCCESS(status))
	//{
	//	DbgPrint("PsRemoveLoadImageNotifyRoutine status = %d", status);
	//}

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
	//KdPrint(("驱动卸载成功\n"));
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
		DbgPrint("创建设备对象失败！\n");
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

	//设置镜像加载回调函数历程
	status = PsSetLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsSetLoadImageNotifyRoutine status = %d", status);
	}

	////设置进程创建回调函数历程
	//status = PsSetCreateProcessNotifyRoutine(ProcessMonitorCallback, FALSE);

	////设置网络监控回调函数历程
	//status = WallRegisterCallouts();
	//if (!NT_SUCCESS(status))
	//{
	//	DbgPrint("WallRegisterCallouts faild! status = %d", status);
	//}

	DbgPrint("驱动加载成功！\n");
	//KdPrint(("驱动加载成功\n"));
	//return status;
	return STATUS_SUCCESS;
}
