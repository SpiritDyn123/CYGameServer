// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
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
		//g_LogicMainThread->Release();�����ⲿmain����ã���Ȼ�Ļ�������Ϊdll�ڲ���wait...����һֱ����ִ�в���ȥ
	}
		break;
	}
	return TRUE;
}

