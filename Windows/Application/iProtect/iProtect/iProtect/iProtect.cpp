// iProtect.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <windows.h>
#include <winternl.h>
#include <conio.h>
//#include <iostream>
#include "LdrInstances.h"


BOOL __stdcall ModifiedEntryPoint(HINSTANCE ModuleInstance, DWORD Reason, LPVOID lpReserved)
{
	if (Reason == DLL_PROCESS_ATTACH)
	{
		MessageBoxA(0, "DLL loading", 0, 0);
	}
	else if (Reason == DLL_PROCESS_DETACH)
	{
		MessageBoxA(0, "DLL exiting", 0, 0);
	}

	return FALSE;
}

BOOL WINAPI _LdrDllHandler(
	_In_ PLDR_DATA_TABLE_ENTRY2 Module,
	_In_ BOOL bIsLoading
)
{
	if (bIsLoading)
	{
		printf("Loading: %ws\n", Module->BaseDllName.Buffer);

		if (StrCmpW(Module->BaseDllName.Buffer, L"Test.dll") == 0)
		{
			Module->LoadCount = 1;
			Module->EntryPoint = &ModifiedEntryPoint;

			printf("拒绝 Loading: %ws\n", Module->BaseDllName.Buffer);
		}
	}
	else
	{
		printf("Unloading: %ws\n", Module->BaseDllName.Buffer);
	}

	return TRUE;
}

int main()
{
	LdrInstance ldr;

	HMODULE hModuleBase = NULL;
	//PWSTR swModuleName = L("mfc120.dll");
	WCHAR m_szModuleName[MAX_PATH] = TEXT("Test.dll");


	if (!ldr.Initialize())
	{
		printf("Failed to initialized.\n");
		ExitProcess(0);
	}

	ldr.RegisterWatchContext(_LdrDllHandler);

	hModuleBase = LoadLibraryW(m_szModuleName); //测试加载

	printf("Module %S loaded in: 0x%.8X\n", m_szModuleName, hModuleBase);

	Sleep(INFINITE);

	return 0;

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
