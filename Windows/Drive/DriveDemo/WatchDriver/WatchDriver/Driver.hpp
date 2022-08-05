#pragma once
#include <ntddk.h>


//定义驱动名称，符号链接名称，事件对象名称
#define DEVICE_NAME L"\\Device\\devMonitorProc"
#define LINK_NAME L"\\??\\slinkMonitorProc"
#define EVENT_NAME L"\\BaseNamedObjects\\MonitorProcEvent" //event对象名称

//缓冲区方式，因为MDL方式不安全
#define IOCTL_NTPROCDRV_GET_PROCINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)


//定义全局的设备对象指针
PDEVICE_OBJECT g_pDeviceObject;

extern "C"
{
	VOID DrvUnload(PDRIVER_OBJECT pDriver);

	NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING reg_path);
}
