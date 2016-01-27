#include "CLog.h"
#include <functional>

CLog::CLog() 
{
}

CLog::~CLog()
{

}

bool CLog::Create(const char *pLogFileName, int RolSize/* = OneGBytes*/, bool bOutputConsole/* = true*/)
{
	_outputConsole = bOutputConsole;

	SetLogOutput(std::bind(&CLog::AsyncOutput, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	_logThread = new CLogToFileThread(pLogFileName, RolSize);

	_logThread->Start();

	return true;
}

void CLog::Release()
{
	if (_logThread == NULL)
		return;

	_logThread->Stop();

	delete _logThread;
	_logThread = NULL;
}

void CLog::SetLogLevel(LogLevel level)
{
	CLoggerSetLogLevel(level);
}

void CLog::SetLogOutput(OutputFunc func)
{
	CLoggerSetOutputFunc(func);
}

void CLog::SetLogFlush(FlushFunc func)
{
	CLoggerSetOutputFlushFunc(func);
}

void CLog::SetLogCustomHead(bool state)
{
	CLoggerSetCustomLogHead(state);
}

void CLog::AsyncOutput(const char *data, int nLen, LogLevel level/* = LogLevel::INFO*/)
{
	if (_outputConsole)
		defaultLogOutput(data, nLen, level);
		
	if (_logThread)
		_logThread->append(data, nLen);
}