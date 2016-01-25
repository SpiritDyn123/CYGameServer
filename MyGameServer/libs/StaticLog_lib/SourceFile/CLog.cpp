#include "CLog.h"

bool g_outputConsole = true;

CLogToFileThread *g_threadLog = NULL;
CLog::CLog() 
{
}

CLog::~CLog()
{

}

bool CLog::Create(const char *pLogFileName, int RolSize/* = OneGBytes*/, bool bOutputConsole/* = true*/)
{
	g_threadLog = new CLogToFileThread(pLogFileName, RolSize);
	
	g_threadLog->Start();

	g_outputConsole = bOutputConsole;

	SetLogOutput(AsyncOutput);

	return true;
}

void CLog::Release()
{
	if (g_threadLog == NULL)
		return;

	g_threadLog->Stop();

	delete g_threadLog;
	g_threadLog = NULL;
}

void CLog::SetLogLevel(LogLevel level)
{
	CLogger::SetLogLevel(level);
}

void CLog::SetLogOutput(CLogger::OutputFunc func)
{
	CLogger::SetOutputFunc(func);
}

void CLog::SetLogFlush(CLogger::FlushFunc func)
{
	CLogger::SetOutputFlushFunc(func);
}

void CLog::AsyncOutput(const char *data, int nLen, LogLevel level/* = LogLevel::INFO*/)
{
	if (g_outputConsole)
		defaultLogOutput(data, nLen, level);
		
	if (g_threadLog)
		g_threadLog->append(data, nLen);
}