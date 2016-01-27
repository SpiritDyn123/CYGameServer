// DynamicLog.cpp : 定义 DLL 应用程序的导出函数。
//


#include "./Src/CDyLog.h"

extern "C" __declspec(dllexport) void CreateLog(ILog **log)
{
	*log = new CDyLog;
}

