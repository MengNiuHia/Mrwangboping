#pragma once
#include <ntddk.h>

extern "C"
{
	// �ܾ���������
	//void DenyLoadDriver(PVOID DriverEntry);
	NTSTATUS DenyLoadDriver(PVOID pImageBase);

	//// �ܾ����� DLL ģ��
	//BOOLEAN DenyLoadDll(PVOID pLoadImageBase);

	// ������ػص�����
	VOID LoadImageNotifyRoutine(_In_ PUNICODE_STRING FullImageName, _In_ HANDLE ProcessId, _In_ PIMAGE_INFO ImageInfo);
}
