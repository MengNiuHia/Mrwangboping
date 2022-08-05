#pragma once

#include <ntddk.h>

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

extern "C"
{
	//进程监视回调函数
	VOID ProcessMonitorCallback(IN HANDLE hParentId, IN HANDLE hProcessId, IN BOOLEAN bCreate);
}
