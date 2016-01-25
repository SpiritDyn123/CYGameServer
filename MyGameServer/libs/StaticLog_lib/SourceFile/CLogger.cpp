#include <Windows.h>
#include "time.h"

#include "CLogger.h"

const char *g_logLevelName[] =
{
	"TRACE",
	"DEBUG",
	"INFO",
	"WARN",
	"ERR",
	"FATAL",
};

const WORD g_logLevelColor[] =
{
	FOREGROUND_GREEN,
   FOREGROUND_GREEN,
   FOREGROUND_GREEN,
   FOREGROUND_RED,
   FOREGROUND_RED,
   FOREGROUND_RED 
};

CLogger::CLogHeader::CLogHeader(LogLevel level, const SourceFile &file, int line)
{
	if (level >= LogLevel::NUM_LOG_LEVELS)
		level = LogLevel::FATAL;
	//(LogLevel 文件名:line)(时间):
	_stream << "(" << g_logLevelName[level] << " " << file._fileName << ":" << line << ")";
	const char *timeStr = formatTime();
	_stream << "(" << timeStr << "):";
	delete timeStr;
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
CLogger::OutputFunc g_output = defaultOutput;
CLogger::FlushFunc g_flush = defaultFlush;


CLogger::CLogger(SourceFile file, int line, LogLevel level /*= LogLevel::INFO*/) :
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

void CLogger::SetLogLevel(LogLevel level)
{
	if (level >= 0 && level < LogLevel::NUM_LOG_LEVELS)
		g_logLevel = level;
}

void CLogger::SetOutputFunc(OutputFunc func)
{
	g_output = func;
}

void CLogger::SetOutputFlushFunc(FlushFunc func)
{
	g_flush = func;
}

