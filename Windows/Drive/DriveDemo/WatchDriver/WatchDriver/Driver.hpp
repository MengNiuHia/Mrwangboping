#pragma once
#include <ntddk.h>


//�����������ƣ������������ƣ��¼���������
#define DEVICE_NAME L"\\Device\\devMonitorProc"
#define LINK_NAME L"\\??\\slinkMonitorProc"
#define EVENT_NAME L"\\BaseNamedObjects\\MonitorProcEvent" //event��������

//��������ʽ����ΪMDL��ʽ����ȫ
#define IOCTL_NTPROCDRV_GET_PROCINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)


//����ȫ�ֵ��豸����ָ��
PDEVICE_OBJECT g_pDeviceObject;

extern "C"
{
	VOID DrvUnload(PDRIVER_OBJECT pDriver);

	NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING reg_path);
}
