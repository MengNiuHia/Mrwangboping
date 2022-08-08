// DirectoryWatch.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <windows.h>
#include <iostream>



#define BUFFER_SIZE 2048

// 存放FILE_NOTIFY_INFORMATION信息的缓冲区
PBYTE pbBuf;
// 用来存放变化的文件名信息
WCHAR wsFileName[MAX_PATH];

int main()
{
	setlocale(LC_ALL, "");//使用wprintf函数前调用setlocale(LC_ALL, "chs")，这样wprintf就可以正常输出中文

	WCHAR pszDirectory[MAX_PATH] = TEXT("E:\\git\\Mrwangboping\\Windows\\Application\\DirectoryWatch\\DirectoryWatch\\TestDirectory\\");
	// 申请缓冲区
	pbBuf = new BYTE[BUFFER_SIZE];

	// 打开目录, 获取文件句柄
	HANDLE hDirectory = ::CreateFile(pszDirectory, FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (INVALID_HANDLE_VALUE == hDirectory)
	{
		;
	}

	BOOL bRet = FALSE;
	DWORD dwRet = 0;

	do
	{
		// 得到 缓冲区
		RtlZeroMemory(pbBuf, BUFFER_SIZE);
		PFILE_NOTIFY_INFORMATION pFileNotifyInfo = (PFILE_NOTIFY_INFORMATION)pbBuf;

		// 设置监控目录
		bRet = ::ReadDirectoryChangesW(hDirectory,
			pFileNotifyInfo,
			BUFFER_SIZE,
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME |			// 修改文件名
			FILE_NOTIFY_CHANGE_DIR_NAME |           // 修改文件夹名
			FILE_NOTIFY_CHANGE_ATTRIBUTES |			// 修改文件属性
			FILE_NOTIFY_CHANGE_LAST_WRITE,			// 最后一次写入
			&dwRet,
			NULL,
			NULL);
		if (FALSE == bRet)
		{
			printf("ReadDirectoryChangesW failed\n");
			break;
		}
		// 判断操作类型并显示
		switch (pFileNotifyInfo->Action)
		{
		case FILE_ACTION_ADDED:
		{
			// 新增文件
			// 由于pFileNotifyInfo不是以零结尾的字符串 所以需要这么处理
			RtlZeroMemory(wsFileName, sizeof(WCHAR) * MAX_PATH);
			memcpy_s(wsFileName, sizeof(WCHAR) * MAX_PATH, pFileNotifyInfo->FileName, pFileNotifyInfo->FileNameLength);
			wprintf(L"[File Added Action] %ls\n", wsFileName);
			break;
		}
		case FILE_ACTION_REMOVED:
		{
			// 移动文件
			// 由于pFileNotifyInfo不是以零结尾的字符串 所以需要这么处理
			RtlZeroMemory(wsFileName, sizeof(WCHAR) * MAX_PATH);
			memcpy_s(wsFileName, sizeof(WCHAR) * MAX_PATH, pFileNotifyInfo->FileName, pFileNotifyInfo->FileNameLength);
			wprintf(L"[File Removed Action] %ls\n", wsFileName);
			break;
		}
		case FILE_ACTION_MODIFIED:
		{
			// 修改文件
			// 由于pFileNotifyInfo不是以零结尾的字符串 所以需要这么处理
			RtlZeroMemory(wsFileName, sizeof(WCHAR) * MAX_PATH);
			memcpy_s(wsFileName, sizeof(WCHAR) * MAX_PATH, pFileNotifyInfo->FileName, pFileNotifyInfo->FileNameLength);
			wprintf(L"[File Modified Action] %ls\n", wsFileName);
			break;
		}
		case FILE_ACTION_RENAMED_OLD_NAME:
		{
			// 重命名文件
			// 由于pFileNotifyInfo不是以零结尾的字符串 所以需要这么处理
			RtlZeroMemory(wsFileName, sizeof(WCHAR) * MAX_PATH);
			memcpy_s(wsFileName, sizeof(WCHAR) * MAX_PATH, pFileNotifyInfo->FileName, pFileNotifyInfo->FileNameLength);
			wprintf(L"[File Removed Action]OldName %ls\n", wsFileName);

			// 下一个FILE_NOTIFY_INFORMATION块就是重命名文件时的新名字
			PFILE_NOTIFY_INFORMATION pNewFileNotifyInfo =
				(PFILE_NOTIFY_INFORMATION)((PBYTE)pFileNotifyInfo + pFileNotifyInfo->NextEntryOffset);
			RtlZeroMemory(wsFileName, sizeof(WCHAR) * MAX_PATH);
			memcpy_s(wsFileName, sizeof(WCHAR) * MAX_PATH, pNewFileNotifyInfo->FileName, pNewFileNotifyInfo->FileNameLength);
			wprintf(L"[File Renamed Action]NewName %ls\n", wsFileName);

			break;
		}
		default:
		{
			break;
		}
		}

	} while (bRet);


	// 关闭句柄, 释放内存
	::CloseHandle(hDirectory);


}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
