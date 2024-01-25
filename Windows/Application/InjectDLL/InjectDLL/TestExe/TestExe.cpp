#include <iostream>
#include <chrono>
#include <thread>
#include <conio.h>
#include <windows.h>

int main()
{
	bool exitFlag = false;

	DWORD currentProcessId = GetCurrentProcessId();
	std::cout << "当前进程ID: " << currentProcessId << std::endl;

	while (!exitFlag)
	{
		// 输出信息
		std::cout << "Hello World!\n";

		// 等待1分钟
		std::this_thread::sleep_for(std::chrono::minutes(1));

		// 检查是否按下了Esc键
		if (_kbhit())
		{
			// 获取按下的键值
			int key = _getch();

			// 判断是否按下了Esc键
			if (key == 27)
			{
				exitFlag = true; // 设置退出标志
			}
		}
	}

	return 0;
}