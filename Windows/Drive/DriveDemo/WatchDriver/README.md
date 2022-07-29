# WatchDriver
驱动主要功能：
1.监控进程创建
2.监控镜像加载
3.通知应用程序

## 开发环境

Windows10 vs2017 SDK版本：10.0.17763.132 WDK版本：10.0.17763.1

## 问题记录
1.使用KmdManager安装并启动驱动时，安装驱动失败，启动失败，错误提示：Error number not found 解决方法：这是因为win10驱动签名必须通过微软认证，可以关闭win10驱动验签即可，如何关闭请百度，如有问题请留言。  2.DbgView未输出DbgPrint或KdPrint日志 解决方法：右键管理员权限启动DbgView，选择Capture菜单中Capture Kernel菜单项,捕获内核程序调试信息。(Capture Win32菜单项,捕获用户程序调试信息); 如果勾选不成功，重命名一下Dbgv.sys，在重新勾选即可。

## 关键函数
PsSetLoadImageNotify例程注册驱动程序提供的回调，随后在加载（或映射到内存）图像（例如，DLL 或 EXE）时通知该回调。
```c
NTSTATUS PsSetLoadImageNotifyRoutine(
  [in] PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
);
```

PsSetCreateProcessNotify例程将驱动程序提供的回调例程添加到或从中创建或删除进程时要调用的例程列表中删除该例程。
```c
NTSTATUS PsSetCreateProcessNotifyRoutine(
  [in] PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine,
  [in] BOOLEAN                        Remove
);
```

IoCreateDevice 例程创建供驱动程序使用的设备对象。
```c
NTSTATUS IoCreateDevice(
  [in]           PDRIVER_OBJECT  DriverObject,
  [in]           ULONG           DeviceExtensionSize,
  [in, optional] PUNICODE_STRING DeviceName,
  [in]           DEVICE_TYPE     DeviceType,
  [in]           ULONG           DeviceCharacteristics,
  [in]           BOOLEAN         Exclusive,
  [out]          PDEVICE_OBJECT  *DeviceObject
);
```

IoCreateSymbolicLink 例程在设备对象名称和设备的用户可见名称之间建立符号链接。
```c
NTSTATUS IoCreateSymbolicLink(
  [in] PUNICODE_STRING SymbolicLinkName,
  [in] PUNICODE_STRING DeviceName
);
```

IoCreateSynchronizationEvent 例程创建或打开一个命名的同步事件，用于序列化两个原本不相关的驱动程序之间的硬件访问。
```c
PKEVENT IoCreateSynchronizationEvent(
  [in]  PUNICODE_STRING EventName,
  [out] PHANDLE         EventHandle
);
```

IoGetCurrentIrpStackLocation 例程返回指向指定 IRP 中调用方的 I/O 堆栈位置的指针。
```c
__drv_aliasesMem PIO_STACK_LOCATION IoGetCurrentIrpStackLocation(
  [in] PIRP Irp
);
```