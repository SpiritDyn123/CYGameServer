// CYSocket.cpp : ���� DLL Ӧ�ó���ĵ���������
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
