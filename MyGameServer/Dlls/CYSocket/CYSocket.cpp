// CYSocket.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ISocketSysterm.h"
#include "CLogger.h"
#include "./SourceFile/CSocketSystem.h"

extern "C" __declspec(dllexport) bool CreateSocketSystem(ISocketSysterm **pSocketSys)
{
	*pSocketSys = g_socketSys;

	return true;
}
