// DynamicLog.cpp : ���� DLL Ӧ�ó���ĵ���������
//


#include "./Src/CDyLog.h"

extern "C" __declspec(dllexport) void CreateLog(ILog **log)
{
	*log = new CDyLog;
}

