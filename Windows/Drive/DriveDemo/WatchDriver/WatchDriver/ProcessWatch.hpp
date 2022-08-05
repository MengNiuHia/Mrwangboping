#pragma once

#include <ntddk.h>

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

extern "C"
{
	//���̼��ӻص�����
	VOID ProcessMonitorCallback(IN HANDLE hParentId, IN HANDLE hProcessId, IN BOOLEAN bCreate);
}
