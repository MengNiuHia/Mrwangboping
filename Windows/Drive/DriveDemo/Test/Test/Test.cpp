
// 载入驱动.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <windows.h>
#include <process.h>
#include <winioctl.h>
#include <TCHAR.h>
using namespace std;
//定义IO控制码
#define IOCTL_NTPROCDRV_GET_PROCINFO CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)

//有缓冲区模式和直接模式
//#define IOCTL_DO_BUFFER CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
//#define IOCTL_IN_DIRECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

//对应驱动里的CALLBACK结构
typedef struct _CALL_BACK
{
	HANDLE hParentId;
	HANDLE hProcessId;
	BOOLEAN bCreate;
}CALLBACKINFO, *PCALLBACKINFO;

typedef struct _Param 
{
	HANDLE pProcessEvent;
	HANDLE pDevice;
	HANDLE hEvent;
}PARAM, *PPARAM;
HANDLE threadHandle;//用来接收驱动消息的线程

unsigned int __stdcall threadWork(LPVOID param)
{
	PPARAM pParam = (PPARAM)param;
	CALLBACKINFO callbackInfo, callbackTemp = { 0 };
	UINT waitState;
	do {
		DWORD dw = WaitForSingleObjectEx(pParam->hEvent, 100, TRUE);
		if (dw == WAIT_IO_COMPLETION) 
		{
			printf_s("WaitForSingleObjectEx pParam->hEvent WAIT_IO_COMPLETION");
			CloseHandle(pParam->pProcessEvent);
			CloseHandle(pParam->pDevice);
			return 1;
		}
		//等待驱动 通知创建进程事件触发 
		waitState = WaitForSingleObject(pParam->pProcessEvent, INFINITE);
		if (waitState == WAIT_OBJECT_0) 
		{
			DWORD nBytesReturn;
			//发送callbackInfo地址，让驱动将进程信息写进来，我们就能在应用层读取创建进程的信息了
			BOOL bRet = DeviceIoControl(pParam->pDevice, IOCTL_NTPROCDRV_GET_PROCINFO, NULL, 0, &callbackInfo, sizeof(callbackInfo), &nBytesReturn, NULL);
			if (bRet)
			{
				if (callbackInfo.hParentId != callbackTemp.hParentId || callbackInfo.hProcessId != callbackTemp.hProcessId || callbackInfo.bCreate != callbackTemp.bCreate) {
					if (callbackInfo.bCreate)
						printf_s("有进程被创建了 PID:%d，父进程 PID:%d\n", (int)callbackInfo.hProcessId, (int)callbackInfo.hParentId);
					else
						printf_s("有进程被终止了 PID:%d，父进程 PID:%d\n", (int)callbackInfo.hProcessId, (int)callbackInfo.hParentId);
					callbackTemp = callbackInfo;
				}
			}
			else {
				printf_s("获取进程信息失败 %d %d\n", bRet, nBytesReturn);
			}
		}
		else 
		{
			printf_s("提示：需要管理员权限\n wait state: 0x%X\n", waitState);
		}
	} while (waitState == WAIT_OBJECT_0);

	CloseHandle(pParam->pProcessEvent);
	CloseHandle(pParam->pDevice);
	return 1;
}

void WINAPI apcFunc(ULONG_PTR dwParam) {
	//Nothing to do in here
}

void onExit() {
	if (threadHandle != INVALID_HANDLE_VALUE)
		QueueUserAPC(apcFunc, threadHandle, NULL);//提醒主线程回收
}

BOOL WINAPI handlerRoutine(DWORD dwCtrlType) {
	char mesg[128];
	_itoa_s(dwCtrlType, mesg, 128, 10);
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:// 当用户按下了CTRL + C, 或者由GenerateConsoleCtrlEvent API发出.
		QueueUserAPC(apcFunc, threadHandle, NULL);
		MessageBoxA(NULL, "CTRL_C received!", "CEvent", MB_OK);
		break;
	case CTRL_CLOSE_EVENT://当试图关闭控制台程序，系统发送关闭消息。
		QueueUserAPC(apcFunc, threadHandle, NULL);
		MessageBoxA(NULL, "CLOSE received!", "CEvent", MB_OK);
		break;
	case CTRL_LOGOFF_EVENT:// 用户退出时，但是不能决定是哪个用户.
		QueueUserAPC(apcFunc, threadHandle, NULL);
		MessageBoxA(NULL, "LOGOFF received!", "CEvent", MB_OK);
		break;
	case CTRL_SHUTDOWN_EVENT://当系统被关闭时.
		QueueUserAPC(apcFunc, threadHandle, NULL);
		MessageBoxA(NULL, "SHUTDOWN received!", "CEvent", MB_OK);
		break;
	default:
		break;
	}

	return 1;
}
int main()
{
	printf_s("提示：\n  需要Release版本 x64 管理员权限\n");

	TCHAR szDriverPath[MAX_PATH];
	TCHAR szLinkName[] = _T("slinkMonitorProc");
	TCHAR * p;

	//可能是目录问题 造成乱码
	GetFullPathName(_T("MyDriver2.sys"), MAX_PATH, szDriverPath, &p);

	//打开驱动程序所控制的设备句柄
	TCHAR sz[256];
	_sntprintf_s(sz, 256, TEXT("\\\\.\\%s"), szLinkName);//就是\\.\linkName
	HANDLE hDevice = CreateFile(sz, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf_s("打开设备失败！%d\n", GetLastError());
		system("pause");
		return -1;
	}

	//打开事件内核对象
	HANDLE hProcessEvent = OpenEvent(SYNCHRONIZE, FALSE, _T("Global\\MonitorProcEvent"));
	if (hProcessEvent == INVALID_HANDLE_VALUE)
	{
		printf_s("获取event handle 出错\n");
		CloseHandle(hProcessEvent);
		CloseHandle(hDevice);
		printf_s("程序结束\n");
		system("pause");
		return 0;
	}


	PARAM param;
	param.pProcessEvent = hProcessEvent;
	param.pDevice = hDevice;
	param.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	printf("registering at exit callback\n");
	atexit(onExit);
	SetConsoleCtrlHandler(handlerRoutine, TRUE);

	threadHandle = (HANDLE)_beginthreadex(NULL, 0, threadWork, (LPVOID)&param, 0, NULL);
	WaitForSingleObjectEx(threadHandle, INFINITE, true);

	printf_s("程序结束\n");
	system("pause");
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
