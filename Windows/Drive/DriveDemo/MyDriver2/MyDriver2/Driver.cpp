#include "Driver.hpp"


HANDLE g_ProcessId = NULL;


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
//���̼��ӻص�����
VOID ProcessMonitorCallback(
	IN HANDLE hParentId,
	IN HANDLE hProcessId,
	IN BOOLEAN bCreate)
{
	//�õ��豸��չ�Ľṹָ��
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)g_pDeviceObject->DeviceExtension;
	//�ȱ�������չ����ǰ���豸��չ�ṹ������Ӧ�ò�
	pDevExt->hPParentId = hParentId;
	pDevExt->hPProcessId = hProcessId;
	pDevExt->bPCreate = bCreate;
	DbgPrint("hProcessId = %d\n", hProcessId);
	//������֪ͨӦ�ò�������򣬵����������� �Ƿ�ȴ������TRUE,��Ҫ��������kewait
	// ����¼���֪ͨ�¼�����ϵͳ�᳢�������¼������Ͼ����ܶ�ĵȴ��� �ڶ� KeClearEvent �� KeResetEvent �ĵ������֮ǰ���¼��������źš� ����¼���ͬ���¼�����ϵͳ�Զ�����¼�֮ǰ�����ȵȴ�һ�Ρ�
	KeSetEvent(pDevExt->pEvent, 0, FALSE);
	//����Ϊ������״̬
	KeClearEvent(pDevExt->pEvent);

}

// �ܾ���������
NTSTATUS DenyLoadDriver(PVOID pImageBase)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMDL pMdl = NULL;
	PVOID pVoid = NULL;
	ULONG ulShellcodeLength = 16;
	UCHAR pShellcode[16] = { 0xB8, 0x22, 0x00, 0x00, 0xC0, 0xC3, 0x90, 0x90,
							0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pImageBase;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
	//�ҵ�DriverEntry�Ļ�ַ
	PVOID pDriverEntry = (PVOID)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.AddressOfEntryPoint);

	pMdl = MmCreateMdl(NULL, pDriverEntry, ulShellcodeLength);
	MmBuildMdlForNonPagedPool(pMdl);
	pVoid = MmMapLockedPages(pMdl, KernelMode);
	RtlCopyMemory(pVoid, pShellcode, ulShellcodeLength);
	MmUnmapLockedPages(pVoid, pMdl);
	IoFreeMdl(pMdl);

	return status;
}


// �ܾ����� DLL ģ��
BOOLEAN DenyLoadDll(PVOID pLoadImageBase)
{
	// DLL�ܾ�����, ����������������ֱ������ڵ㷵�ؾܾ�������Ϣ. �����ﲻ��ж��DLL��Ч��.
	// ���ļ�ͷ ǰ0x200 �ֽ���������

	ULONG ulDataSize = 0x200;
	// ���� MDL ��ʽ�޸��ڴ�
	PMDL pMdl = MmCreateMdl(NULL, pLoadImageBase, ulDataSize);
	if (NULL == pMdl)
	{
		DbgPrint("MmCreateMdl");
		return FALSE;
	}
	MmBuildMdlForNonPagedPool(pMdl);
	PVOID pVoid = MmMapLockedPages(pMdl, KernelMode);
	if (NULL == pVoid)
	{
		IoFreeMdl(pMdl);
		DbgPrint("MmMapLockedPages");
		return FALSE;
	}
	// ����
	RtlZeroMemory(pVoid, ulDataSize);
	// �ͷ� MDL
	MmUnmapLockedPages(pVoid, pMdl);
	IoFreeMdl(pMdl);

	return TRUE;
}

// �ص�����
VOID LoadImageNotifyRoutine(_In_ PUNICODE_STRING FullImageName, _In_ HANDLE ProcessId, _In_ PIMAGE_INFO ImageInfo)
{
	// ��ʾ����ģ����Ϣ
	DbgPrint("[ProcessId = %d][FullImageName = %wZ][ImageSize = %d][ImageBase = 0x%p]\n", ProcessId, FullImageName, ImageInfo->ImageSize, ImageInfo->ImageBase);

	// �ܾ�����ָ��ģ��
	if (NULL != wcsstr(FullImageName->Buffer, L"Driver3.sys") ||
		NULL != wcsstr(FullImageName->Buffer, L"Test.dll"))
	{
		// Driver
		if (0 == ProcessId)
		{
			DbgPrint("Deny Load Driver\n");
			DenyLoadDriver(ImageInfo->ImageBase);
		}
		// Dll
		else
		{
			DbgPrint("Deny Load DLL\n");
			DenyLoadDll(ImageInfo->ImageBase);
		}
	}

	//�ܾ�Everything.exe����dll
	if (NULL != wcsstr(FullImageName->Buffer, L"Everything.exe"))
	{
		DbgPrint("����Everything���� ����id = %d\n", ProcessId);
		//����һ�½���ID
		g_ProcessId = ProcessId;
	}

	//��ֹ�ý���ID����dll
	if (g_ProcessId == ProcessId && NULL != wcsstr(FullImageName->Buffer, L".dll"))
	{
		DbgPrint("Deny Load DLL\n");
		DenyLoadDll(ImageInfo->ImageBase);
	}

}

VOID DrvUnload(PDRIVER_OBJECT pDriver)
{
	NTSTATUS status;
	status  = PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsRemoveLoadImageNotifyRoutine status = %d", status);
	}

	////�Ƴ�֪ͨ����
	//PsSetCreateProcessNotifyRoutine(ProcessMonitorCallback, TRUE);

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
	KdPrint(("����ж�سɹ�\n"));
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

	status = PsSetLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsSetLoadImageNotifyRoutine status = %d", status);
	}

	////���ûص���������
	//status = PsSetCreateProcessNotifyRoutine(ProcessMonitorCallback, FALSE);

	DbgPrint("�������سɹ���\n");
	KdPrint(("�������سɹ�\n"));
	return status;
}
