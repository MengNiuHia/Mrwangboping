#include "ProcessWatch.hpp"

extern PDEVICE_OBJECT g_pDeviceObject;

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

	if (bCreate)
		DbgPrint("创建进程 进程ID =  = %d\n", hProcessId);
	else
		DbgPrint("退出进程 进程ID =  = %d\n", hProcessId);
	
	//触发，通知应用层监听程序，第三个参数是 是否等待，如果TRUE,需要驱动调用kewait
	// 如果事件是通知事件，则系统会尝试满足事件对象上尽可能多的等待。 在对 KeClearEvent 或 KeResetEvent 的调用清除之前，事件将保持信号。 如果事件是同步事件，则系统自动清除事件之前，会先等待一次。
	KeSetEvent(pDevExt->pEvent, 0, FALSE);
	//设置为非授信状态
	KeClearEvent(pDevExt->pEvent);

}