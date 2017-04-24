// CrashCapture.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h> //声明快照函数文件
#include "stdio.h"
#include <cstring>
#include <time.h>

// 根据进程ID， 返回指定进程下"第一个"窗口的窗口句柄
// 注: 此程序还不够完善, 因为指定进程下可能有多个窗口
HWND GetWindowHandleByPID(DWORD dwProcessID)
{
	HWND h = GetTopWindow(0);
	while (h)
	{
		DWORD pid = 0;
		DWORD dwTheardId = GetWindowThreadProcessId(h, &pid);

		if (dwTheardId != 0)
		{
			if (pid == dwProcessID /*your process id*/)
			{
				// here h is the handle to the window
				return h;
			}
		}

		h = GetNextWindow(h, GW_HWNDNEXT);
	}

	return NULL;
}

void wcharTochar(const wchar_t *wchar, char *chr, int length)
{
	WideCharToMultiByte(CP_ACP, 0, wchar, -1, chr, length, NULL, NULL);
}

int main(int argc, char *argv[])
{
	PROCESSENTRY32 pe32;

	//在使用这个结构之前，先设置它的大小
	pe32.dwSize = sizeof(pe32);
	HANDLE hProcessSnap;
	char sbuf[100];
	char pName[MAX_PATH];
	FILE *fLog = NULL;
	time_t rawtime=time(NULL);
	struct tm* timeinfo;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: CrashCapture.exe x265.exe 1.txt/n");
		exit(1);
	}

	while (1)
	{
		//给系统内的所有进程拍一个快照
		hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		//Includes the process list in the snapshot
		if (hProcessSnap == INVALID_HANDLE_VALUE)
		{
			printf("CreateToolhelp32Snapshot 调用失败! n");
			return -1;
		}

		//遍历进程快照，轮流显示每个进程信息
		BOOL bMore = ::Process32First(hProcessSnap, &pe32);
		while (bMore)
		{
			wcharTochar(pe32.szExeFile, pName, sizeof(pName));

			if (_stricmp(pName, "werfault.exe") == 0)
			{
				//printf(" 进程名称为:%s\n", pName);
				//printf(" 进程ID为:%u \n\n", pe32.th32ProcessID);

				HWND hwnd = GetWindowHandleByPID(pe32.th32ProcessID);
				if (hwnd)
				{
					char szText[256] = { 0 };
					GetWindowText(hwnd, (LPWSTR)szText, 256);
					wcharTochar((wchar_t *)szText, pName, sizeof(pName));
					// 自己崩溃程序的"进程名"
					if (_stricmp(pName, argv[1]) == 0)
					{
						timeinfo = localtime(&rawtime);
						printf("Text: %s crashed at %d/%d/%d  %d:%d:%d\n", pName, (1900 + timeinfo->tm_year), (1 + timeinfo->tm_mon), timeinfo->tm_mday,
							timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
						sprintf(sbuf, "Text: %s crashed at %d/%d/%d  %d:%d:%d\n", pName, (1900 + timeinfo->tm_year), (1 + timeinfo->tm_mon), timeinfo->tm_mday,
							timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
						fLog = fopen(argv[2], "wb");
						if (NULL == fLog)
						{
							fprintf(stderr, "Unable to open the file to save log %s.\n", argv[2]);
							goto exitmain;
						}
						fputs(sbuf, fLog);
						fclose(fLog);
						// 关闭"xx程序已停止"提示窗口
						SendMessage(hwnd, WM_CLOSE, NULL, NULL);
					}

				}
			}

			//遍历下一个
			bMore = ::Process32Next(hProcessSnap, &pe32);
		}
		//清除snapshot对象
		::CloseHandle(hProcessSnap);
	}
exitmain:
	if (fLog != 0)     fclose(fLog);

	return 0;
}
