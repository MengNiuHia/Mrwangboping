#include "LoadImageWatch.hpp"
#include <ntimage.h>

HANDLE g_ProcessId = NULL;



//BOOLEAN VxkCopyMemory(PVOID pDestination, PVOID pSourceAddress, SIZE_T SizeOfCopy)
//{
//	PMDL pMdl = NULL;
//	PVOID pSafeAddress = NULL;
//	pMdl = IoAllocateMdl(pSourceAddress, (ULONG)SizeOfCopy, FALSE, FALSE, NULL);
//	if (!pMdl) return FALSE;
//	__try
//	{
//		MmProbeAndLockPages(pMdl, KernelMode, IoReadAccess);
//	}
//	__except (EXCEPTION_EXECUTE_HANDLER)
//	{
//		IoFreeMdl(pMdl);
//		return FALSE;
//	}
//	pSafeAddress = MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority);
//	if (!pSafeAddress) return FALSE;
//	RtlCopyMemory(pDestination, pSafeAddress, SizeOfCopy);
//	MmUnlockPages(pMdl);
//	IoFreeMdl(pMdl);
//	return TRUE;
//}

BOOLEAN VxkCopyMemory(PVOID pDriverEntry, PVOID pSourceAddress, SIZE_T SizeOfCopy)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMDL pMdl = NULL;
	PVOID pVoid = NULL;

	pMdl = MmCreateMdl(NULL, pDriverEntry, SizeOfCopy);
	if (!pMdl)
	{
		DbgPrint("MmCreateMdl failed\n");
		return FALSE;
	}

	MmBuildMdlForNonPagedPool(pMdl);
	pVoid = MmMapLockedPages(pMdl, KernelMode);
	RtlCopyMemory(pVoid, pSourceAddress, SizeOfCopy);
	MmUnmapLockedPages(pVoid, pMdl);
	IoFreeMdl(pMdl);
	return TRUE;
}

VOID UnicodeToChar(PUNICODE_STRING dst, char *src)
{
	ANSI_STRING string;
	RtlUnicodeStringToAnsiString(&string, dst, TRUE);
	strcpy(src, string.Buffer);
	RtlFreeAnsiString(&string);
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
//
//
//// �ܾ����� DLL ģ��
//BOOLEAN DenyLoadDll(PVOID pLoadImageBase)
//{
//	// DLL�ܾ�����, ����������������ֱ������ڵ㷵�ؾܾ�������Ϣ. �����ﲻ��ж��DLL��Ч��.
//	// ���ļ�ͷ ǰ0x200 �ֽ���������
//
//	ULONG ulDataSize = 0x200;
//	// ���� MDL ��ʽ�޸��ڴ�
//	PMDL pMdl = MmCreateMdl(NULL, pLoadImageBase, ulDataSize);
//	if (NULL == pMdl)
//	{
//		DbgPrint("MmCreateMdl");
//		return FALSE;
//	}
//	MmBuildMdlForNonPagedPool(pMdl);
//	PVOID pVoid = MmMapLockedPages(pMdl, KernelMode);
//	if (NULL == pVoid)
//	{
//		IoFreeMdl(pMdl);
//		DbgPrint("MmMapLockedPages");
//		return FALSE;
//	}
//	// ����
//	RtlZeroMemory(pVoid, ulDataSize);
//	// �ͷ� MDL
//	MmUnmapLockedPages(pVoid, pMdl);
//	IoFreeMdl(pMdl);
//
//	return TRUE;
//}

//PVOID GetDriverEntryByImageBase(PVOID ImageBase)
//{
//	PIMAGE_DOS_HEADER pDOSHeader;
//	PIMAGE_NT_HEADERS64 pNTHeader;
//	PVOID pEntryPoint;
//	pDOSHeader = (PIMAGE_DOS_HEADER)ImageBase;
//	pNTHeader = (PIMAGE_NT_HEADERS64)((ULONG64)ImageBase + pDOSHeader->e_lfanew);
//	pEntryPoint = (PVOID)((ULONG64)ImageBase + pNTHeader->OptionalHeader.AddressOfEntryPoint);
//	return pEntryPoint;
//}

PVOID GetDriverEntryByImageBase(PVOID ImageBase)
{
	PVOID pDriverEntry;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
	//�ҵ�DriverEntry�Ļ�ַ
	pDriverEntry = (PVOID)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.AddressOfEntryPoint);
	return pDriverEntry;
}

void DenyLoadDriver1(PVOID DriverEntry)
{
	UCHAR fuck[] = "\xB8\x22\x00\x00\xC0\xC3";
	UCHAR pShellcode[16] = { 0xB8, 0x22, 0x00, 0x00, 0xC0, 0xC3, 0x90, 0x90,
							0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	BOOLEAN bRet = VxkCopyMemory(DriverEntry, pShellcode, sizeof(pShellcode));
	if (!bRet)
	{
		DbgPrint("VxkCopyMemory failed\n");
	}
}

void DenyLoadDll1(PVOID DriverEntry)
{
	UCHAR pShellcode[16] = { 0xB8, 0x22, 0x00, 0x00, 0xC0, 0xC3, 0x90, 0x90,
							0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	BOOLEAN bRet = VxkCopyMemory(DriverEntry, pShellcode, sizeof(pShellcode));
	if (!bRet)
	{
		DbgPrint("VxkCopyMemory failed\n");
	}
}


//void DenyLoadDriver(PVOID DriverEntry)
//{
//	UCHAR fuck[] = "\xB8\x22\x00\x00\xC0\xC3";
//
//	BOOLEAN bRet = VxkCopyMemory(DriverEntry, fuck, sizeof(fuck));
//	if (!bRet)
//	{
//		DbgPrint("VxkCopyMemory failed\n");
//	}
//}


// �ص�����
VOID LoadImageNotifyRoutine(_In_ PUNICODE_STRING FullImageName, _In_ HANDLE ProcessId, _In_ PIMAGE_INFO ImageInfo)
{
	// ��ʾ����ģ����Ϣ
	DbgPrint("[ProcessId = %d][FullImageName = %wZ][ImageSize = %d][ImageBase = 0x%p]\n", ProcessId, FullImageName, ImageInfo->ImageSize, ImageInfo->ImageBase);

	PVOID pDrvEntry;

	pDrvEntry = GetDriverEntryByImageBase(ImageInfo->ImageBase);

	// �ܾ�����ָ��ģ��
	if (NULL != wcsstr(FullImageName->Buffer, L"Driver2.sys") ||
		NULL != wcsstr(FullImageName->Buffer, L"Test.dll"))
	{
		// Driver
		if (0 == ProcessId)
		{
			DbgPrint("Deny Load Driver\n");
			//DenyLoadDriver(ImageInfo->ImageBase);
			DenyLoadDriver1(pDrvEntry);
		}
		//// Dll
		//else
		//{
		//	DbgPrint("Deny Load DLL\n");
		//	DenyLoadDll(ImageInfo->ImageBase);
		//}
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
		DenyLoadDll1(pDrvEntry);
	}

}