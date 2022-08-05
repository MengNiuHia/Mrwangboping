#include <ntifs.h>
#include <ntimage.h>
#include <devioctl.h>

#include "Driver.hpp"
#include "ProcessWatch.hpp"
#include "WFPNetworkWatch.hpp"
#include "LoadImageWatch.hpp"

//ֱ�ӷ��������Ϣ	
NTSTATUS DispatchCreateClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//��ɴ�����
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//I/O��ǲ���̴���
NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("monitorProc : DispatchIotcl...\n");
	//Ĭ�Ϸ���ʧ��
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	//ȡ�ô�IRP��I/O��ջָ��
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	//ȡ���豸��չ�ṹָ��
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	//��ȡI/O���ƴ���
	ULONG uIoContrlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	//ȡ��I/O��������ָ����䳤��
	pCallbackInfo pCallbackInfoBuff = (pCallbackInfo)pIrp->AssociatedIrp.SystemBuffer;
	ULONG uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;//���뻺��������
	ULONG uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;//�������������
	switch (uIoContrlCode)
	{
	case IOCTL_NTPROCDRV_GET_PROCINFO:   //���û����򷵻����¼������Ľ�����Ϣ
	{
		if (uOutSize >= sizeof(CallbackInfo)) //�������������Ҫ���ڷ��ؽṹ��С
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
	//�������
	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

VOID DrvUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status;

	////�Ƴ�������֪ͨ����
	//status =  WallUnRegisterCallouts();

	//�Ƴ��������֪ͨ����
	status  = PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);

	////�Ƴ����̴���֪ͨ����
	//status = PsSetCreateProcessNotifyRoutine(ProcessMonitorCallback, TRUE);
	//if (!NT_SUCCESS(status))
	//{
	//	DbgPrint("PsRemoveLoadImageNotifyRoutine status = %d", status);
	//}

	//�ر�event
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)g_pDeviceObject->DeviceExtension;
	KeClearEvent(pDevExt->pEvent);
	ZwClose(pDevExt->hEventHandle);
	//ɾ���豸���ӷ���
	UNICODE_STRING strLink;
	RtlInitUnicodeString(&strLink, LINK_NAME);
	IoDeleteSymbolicLink(&strLink);
	IoDeleteDevice(pDriver->DeviceObject); //ɾ���豸����

	DbgPrint("����ж�سɹ���\n");
	//KdPrint(("����ж�سɹ�\n"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING reg_path)
{
	NTSTATUS status = STATUS_SUCCESS;
	//��ʼ����������
	pDriver->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;
	pDriver->DriverUnload = DrvUnload;


	//����������ʼ���豸����
	UNICODE_STRING strDevName;
	RtlInitUnicodeString(&strDevName, DEVICE_NAME);
	//�����豸����
	PDEVICE_OBJECT pDevObj;
	status = IoCreateDevice(
		pDriver,
		sizeof(DEVICE_EXTENSION),//Ϊ�豸��չ�ṹ����ռ�
		&strDevName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&pDevObj
	);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("�����豸����ʧ�ܣ�\n");
		return status;
	}
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	//������������
	UNICODE_STRING strLinkName;
	RtlInitUnicodeString(&strLinkName, LINK_NAME);
	//���� ����
	status = IoCreateSymbolicLink(&strLinkName, &strDevName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	//���豸ָ�뱣�浽ȫ�ֱ����У��������ʹ��
	g_pDeviceObject = pDevObj;
	//Ϊ���ܹ������û�����̣������¼�����
	UNICODE_STRING szpEventString;
	RtlInitUnicodeString(&szpEventString, EVENT_NAME);
	//����һ��event����
	pDevExt->pEvent = IoCreateSynchronizationEvent(&szpEventString, &pDevExt->hEventHandle);
	//���÷�����״̬
	KeClearEvent(pDevExt->pEvent);

	//���þ�����ػص���������
	status = PsSetLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsSetLoadImageNotifyRoutine status = %d", status);
	}

	////���ý��̴����ص���������
	//status = PsSetCreateProcessNotifyRoutine(ProcessMonitorCallback, FALSE);

	////���������ػص���������
	//status = WallRegisterCallouts();
	//if (!NT_SUCCESS(status))
	//{
	//	DbgPrint("WallRegisterCallouts faild! status = %d", status);
	//}

	DbgPrint("�������سɹ���\n");
	//KdPrint(("�������سɹ�\n"));
	//return status;
	return STATUS_SUCCESS;
}
