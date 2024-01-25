#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <tchar.h>

#define DEF_DLL_NAME (L"TestDLL.dll")      //定义要卸载的dll名

// DLL注入函数
BOOL InjectDLL(DWORD dwProcessId, const char* szDllPath)
{
	// 打开目标进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (hProcess == NULL)
	{
		std::cout << "Failed to open target process." << std::endl;
		return FALSE;
	}

	// 在目标进程中分配内存空间
	LPVOID pRemoteBuf = VirtualAllocEx(hProcess, NULL, strlen(szDllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	if (pRemoteBuf == NULL)
	{
		std::cout << "Failed to allocate memory in target process." << std::endl;
		CloseHandle(hProcess);
		return FALSE;
	}

	// 将DLL路径写入目标进程的内存空间
	if (!WriteProcessMemory(hProcess, pRemoteBuf, szDllPath, strlen(szDllPath) + 1, NULL))
	{
		std::cout << "Failed to write DLL path to target process." << std::endl;
		VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return FALSE;
	}

	// 创建远程线程，调用LoadLibrary函数加载DLL
	HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA"), pRemoteBuf, 0, NULL);
	if (hRemoteThread == NULL)
	{
		std::cout << "Failed to create remote thread in target process." << std::endl;
		VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return FALSE;
	}

	// 等待远程线程结束
	WaitForSingleObject(hRemoteThread, INFINITE);

	std::cout << "DLL injected successfully." << std::endl;

	// 清理操作
	VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
	CloseHandle(hRemoteThread);
	CloseHandle(hProcess);

	return TRUE;
}

DWORD FindProcessID(LPCTSTR szProcessName)
{
	DWORD dwPID = 0xFFFFFFFF;								//初始化PID为0xFFFFFFFF
	HANDLE hSnapShot = INVALID_HANDLE_VALUE; //初始化快照句柄为INVALID_HANDLE_VALUE
	PROCESSENTRY32 pe;	//定义一个存放 快照进程信息 的一个结构体
	//1.获取当前系统进程快照
	pe.dwSize = sizeof(PROCESSENTRY32);
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	//2.查找进程
	Process32First(hSnapShot, &pe);
	do
	{
		if (!_tcsicmp(szProcessName, (LPCTSTR)pe.szExeFile))     //比较进程名
		{
			dwPID = pe.th32ProcessID;
			break;
		}
	} while (Process32Next(hSnapShot, &pe));     //循环查找
	//关闭句柄并返回
	CloseHandle(hSnapShot);
	return dwPID;
}

BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;      //用来储存特权值信息的结构体
	HANDLE hToken;				  //Token的句柄
	LUID luid;

	//1.打开一个与进程相关联的访问token
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		_tprintf(L"OpenProcessToken failed: %u\n", GetLastError());
		return FALSE;
	}

	//2.查找系统权限值并储存到luid中
	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{

		_tprintf(L"LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	//3.将这些都存入到TOKEN_PRIVILEGES结构体中
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)  //根据判断本函数的第二个参数设置属性
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
	}

	//4.提权
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
		_tprintf(L"AdjustTokenPrivileges error:%u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		_tprintf(L"The token dose not have the specified privilege.\n");
		return FALSE;
	}
	return TRUE;
}

BOOL EjectDll(DWORD dwPID, LPCTSTR szDllName)
{
	BOOL bMore = FALSE, bFound = FALSE;
	HANDLE hSnapshot, hProcess, hThread;
	HMODULE hModule = NULL;
	MODULEENTRY32 me = { sizeof(me) }; //定义一个用于储存模块快照的结构体
	LPTHREAD_START_ROUTINE pThreadProc;

	//1.
	//dwPID=notepad进程ID
	//使用TH32CS_SNAPMODULE参数
	//获取加载到notepad进程的DLL名称
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);

	//此函数检索与进程相关联的第一个模块的信息
	bMore = Module32First(hSnapshot, &me);

	for (; bMore; bMore = Module32Next(hSnapshot, &me))   //bMore用于判断该进程的模块快照是否还有，bFound用于判断是否找到了我们想要卸载的dll模块
	{
		if (!_tcsicmp((LPCTSTR)me.szModule, szDllName) || !_tcsicmp((LPCTSTR)me.szExePath, szDllName))
		{
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
	{
		CloseHandle(hSnapshot);
		return FALSE;
	}
	//2. 通过进程PID获取进程句柄
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		_tprintf(L"OpenProcess(%d) failed!!![%d]\n", dwPID, GetLastError);
		return FALSE;
	}
	//3. 获取FreeLibrary函数的地址
	hModule = GetModuleHandle(L"kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "FreeLibrary");
	//4.创建线程来执行FreeLibrary(modBaseAddr要卸载的dll模块基址)
	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.modBaseAddr, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	CloseHandle(hSnapshot);

	return TRUE;
}

int main()
{
	DWORD dwProcessId; // 目标进程ID
	const char* szDllPath = "E:\\git\\Mrwangboping\\Windows\\Application\\InjectDLL\\InjectDLL\\Debug\\TestDLL.dll"; // 待注入的DLL路径

	std::cout << "Enter the process ID to inject DLL: ";
	std::cin >> dwProcessId;

	if (InjectDLL(dwProcessId, szDllPath))
	{
		std::cout << "DLL injection succeeded." << std::endl;

		std::cout << "Press any key to unload the DLL...";
		std::cin.ignore();

		/*更改权限 管理员权限*/
		if (!SetPrivilege(SE_DEBUG_NAME, TRUE))
			return 1;

		/*调用函数卸载dll模块*/
		if (EjectDll(dwProcessId, DEF_DLL_NAME))
			std::cout << "DLL unloaded successfully." << std::endl;
		else
			std::cout << "Failed to unload DLL." << std::endl;

	}
	else
	{
		std::cout << "DLL injection failed." << std::endl;
	}

	return 0;
}