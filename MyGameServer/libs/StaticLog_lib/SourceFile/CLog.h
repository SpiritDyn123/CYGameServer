#ifndef CLOG_H
#define CLOG_H

#include <Windows.h>
#include <iostream>
#include "CLogger.h"
#include "CLogToFileThread.h"
#include "ILog.h"

class CLog 
{
public:
	CLog();

	~CLog();

public:

	bool Create(const char *pLogFileName, int RolSize = OneGBytes, bool bOutputConsole = true);

	void Release();

	void SetLogLevel(LogLevel level);

	void SetLogOutput(CLogger::OutputFunc func);

	void SetLogFlush(CLogger::FlushFunc func);

public:
	static void AsyncOutput(const char *data, int nLen, LogLevel level = LogLevel::INFO);

private:
	
};

#endif