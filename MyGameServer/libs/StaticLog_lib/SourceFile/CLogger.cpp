#include <Windows.h>
#include "time.h"

#include "CLogger.h"

const WORD g_logLevelColor[] =
{
	FOREGROUND_GREEN,
	FOREGROUND_GREEN,
	FOREGROUND_GREEN,
	FOREGROUND_RED,
	FOREGROUND_RED,
	FOREGROUND_RED
};


//多线程情况下打印终端可能会错位，因为没有加锁
void defaultOutput(const char* msg, int len, LogLevel level = LogLevel::INFO)
{
	HANDLE hCout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hCout)
	{
		SetConsoleTextAttribute(hCout, g_logLevelColor[level]);
	}

	fwrite(msg, 1, len, stdout);

	SetConsoleTextAttribute(hCout, 7);//默认白色
}

void defaultFlush()
{
	fflush(stdout);
}

void defaultLogOutput(const char* msg, int len, LogLevel level/* = LogLevel::INFO*/)
{
	defaultOutput(msg, len, level);
}

void defaultLogFlush()
{
	defaultFlush();
}

LogLevel g_logLevel = LogLevel::INFO;
OutputFunc g_output = defaultOutput;
FlushFunc g_flush = defaultFlush;
bool g_customLogHead = false;

CLogger::CLogHeader::CLogHeader(LogLevel level, char *file, int line)
{
	if (!g_customLogHead)
	{
		if (level >= LogLevel::NUM_LOG_LEVELS)
			level = LogLevel::FATAL;

		_stream << CLogHelper::GenerateLogHead(file, line, level).c_str();
	}
}

const char * CLogger::CLogHeader::formatTime()
{
	time_t tNow = time(NULL);
	tm t;
	localtime_s(&t, &tNow);

	char *timeStr = new char[32];
	sprintf_s(timeStr, 32, "%4d-%02d-%02d %02d:%02d:%02d"
		, t.tm_year + 1900
		, t.tm_mon + 1
		, t.tm_mday
		, t.tm_hour
		, t.tm_min
		, t.tm_sec);

	return timeStr;
}

void CLoggerSetLogLevel(LogLevel level)
{
	g_logLevel = level;
}

void CLoggerSetOutputFunc(OutputFunc func)
{
	g_output = func;
}

void CLoggerSetOutputFlushFunc(FlushFunc func)
{
	g_flush = func;
}

LogLevel CLoggerLogLevel() 
{ 
	return g_logLevel;
}

void CLoggerSetCustomLogHead(bool state)
{
	g_customLogHead = state;
}

CLogger::CLogger(char *file, int line, LogLevel level /*= LogLevel::INFO*/) :
	_header(level, file, line),
	_level(level)
{

}

CLogger::~CLogger()
{
	Stream() << "\n";

	CLogStream::Buffer &buff = Stream().buffer();

	if (g_output)
		g_output(buff.data(), buff.length(), _level);

	if (g_flush)
		g_flush();
}

