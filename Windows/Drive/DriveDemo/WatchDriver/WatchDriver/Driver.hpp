#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <ntimage.h>
#include <devioctl.h>


//�����������ƣ������������ƣ��¼���������
#define DEVICE_NAME L"\\Device\\devMonitorProc"
#define LINK_NAME L"\\??\\slinkMonitorProc"
#define EVENT_NAME L"\\BaseNamedObjects\\MonitorProcEvent" //event��������

//��������ʽ����ΪMDL��ʽ����ȫ
#define IOCTL_NTPROCDRV_GET_PROCINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
typedef struct _CallbackInfo
{
	HANDLE hParentId;
	HANDLE hProcessId;
	BOOLEAN bCreate;
}CallbackInfo, *pCallbackInfo;

//�豸������չ�ṹ�����һЩ��Ҫ��Ϣ
typedef struct _DEVICE_EXTENSION
{
	HANDLE hEventHandle;//�¼�������
	PKEVENT pEvent;// ���ں��ں�ͨ���¼��Ķ���ָ��
	HANDLE hPParentId;   //�ڻص������б��������Ϣ,���ݸ��û���
	HANDLE hPProcessId;
	BOOLEAN bPCreate; //true��ʾ���̱�����,false��ʾ���̱�ɾ��
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;
//����ȫ�ֵ��豸����ָ��
PDEVICE_OBJECT g_pDeviceObject;

extern "C"
{
	VOID DrvUnload(PDRIVER_OBJECT pDriver);

	NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING reg_path);
}
