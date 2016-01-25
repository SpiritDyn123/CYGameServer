// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "ITest.h"

bool runing;
HANDLE g_thread;
DWORD WINAPI ThreadFunc(LPVOID lp)
{
	while (runing)
	{

	}
	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
							   runing = true;
							   g_thread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);
	}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
		{
			runing = false;
			DWORD reslut = WaitForSingleObject(g_thread, 1000);
			CloseHandle(g_thread);
		}
		break;
	}
		break;
	}
	return TRUE;
}

