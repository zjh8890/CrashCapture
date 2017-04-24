// CrashCapture.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h> //�������պ����ļ�
#include "stdio.h"
#include <cstring>
#include <time.h>

// ���ݽ���ID�� ����ָ��������"��һ��"���ڵĴ��ھ��
// ע: �˳��򻹲�������, ��Ϊָ�������¿����ж������
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

	//��ʹ������ṹ֮ǰ�����������Ĵ�С
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
		//��ϵͳ�ڵ����н�����һ������
		hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		//Includes the process list in the snapshot
		if (hProcessSnap == INVALID_HANDLE_VALUE)
		{
			printf("CreateToolhelp32Snapshot ����ʧ��! n");
			return -1;
		}

		//�������̿��գ�������ʾÿ��������Ϣ
		BOOL bMore = ::Process32First(hProcessSnap, &pe32);
		while (bMore)
		{
			wcharTochar(pe32.szExeFile, pName, sizeof(pName));

			if (_stricmp(pName, "werfault.exe") == 0)
			{
				//printf(" ��������Ϊ:%s\n", pName);
				//printf(" ����IDΪ:%u \n\n", pe32.th32ProcessID);

				HWND hwnd = GetWindowHandleByPID(pe32.th32ProcessID);
				if (hwnd)
				{
					char szText[256] = { 0 };
					GetWindowText(hwnd, (LPWSTR)szText, 256);
					wcharTochar((wchar_t *)szText, pName, sizeof(pName));
					// �Լ����������"������"
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
						// �ر�"xx������ֹͣ"��ʾ����
						SendMessage(hwnd, WM_CLOSE, NULL, NULL);
					}

				}
			}

			//������һ��
			bMore = ::Process32Next(hProcessSnap, &pe32);
		}
		//���snapshot����
		::CloseHandle(hProcessSnap);
	}
exitmain:
	if (fLog != 0)     fclose(fLog);

	return 0;
}
