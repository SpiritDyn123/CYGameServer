// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include "CLogger.h"

#pragma comment(lib, "..\\..\\..\\Bin\\StaticLog_lib.lib")

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
	}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
		//g_LogicMainThread->Release();放在外部main里调用，不然的话，会因为dll内部的wait...函数一直阻塞执行不下去
	}
		break;
	}
	return TRUE;
}

