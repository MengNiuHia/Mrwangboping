#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <ntimage.h>
#include <devioctl.h>


//定义驱动名称，符号链接名称，事件对象名称
#define DEVICE_NAME L"\\Device\\devMonitorProc"
#define LINK_NAME L"\\??\\slinkMonitorProc"
#define EVENT_NAME L"\\BaseNamedObjects\\MonitorProcEvent" //event对象名称

//缓冲区方式，因为MDL方式不安全
#define IOCTL_NTPROCDRV_GET_PROCINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
typedef struct _CallbackInfo
{
	HANDLE hParentId;
	HANDLE hProcessId;
	BOOLEAN bCreate;
}CallbackInfo, *pCallbackInfo;

//设备对象扩展结构，存放一些必要信息
typedef struct _DEVICE_EXTENSION
{
	HANDLE hEventHandle;//事件对象句柄
	PKEVENT pEvent;// 用于和内核通信事件的对象指针
	HANDLE hPParentId;   //在回调函数中保存进程信息,传递给用户层
	HANDLE hPProcessId;
	BOOLEAN bPCreate; //true表示进程被创建,false表示进程被删除
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;
//定义全局的设备对象指针
PDEVICE_OBJECT g_pDeviceObject;

extern "C"
{
	VOID DrvUnload(PDRIVER_OBJECT pDriver);

	NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING reg_path);
}
