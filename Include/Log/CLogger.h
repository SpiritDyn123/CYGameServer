#ifndef CLOGGER_H
#define CLOGGER_H

#include <iostream>
#include "CLogStream.h"
#include "ILog.h"
#include <functional>
//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/*
log文件格式：程序名_时间.log
log行数据格式:(LogLevel 文件名:line)(时间):字符串\n
文件滚动大小为1G，时间为每天0点
*/
/************************************************************************/
//////////////////////////////////////////////////////////////////////////

void defaultLogOutput(const char* msg, int len, LogLevel level = LogLevel::INFO);

void defaultLogFlush();

typedef std::function<void(const char*, int, LogLevel)> OutputFunc;
typedef std::function<void()> FlushFunc;

void CLoggerSetLogLevel(LogLevel level);
void CLoggerSetOutputFunc(OutputFunc func);
void CLoggerSetOutputFlushFunc(FlushFunc func);
LogLevel CLoggerLogLevel();
void CLoggerSetCustomLogHead(bool state);

class CLogger
{
public:

	class CLogHeader
	{
	public:
		CLogHeader(LogLevel level, char *file, int line);

		const char * formatTime();

		CLogStream _stream;
	};

public:
	CLogger(char *file, int line, LogLevel level = LogLevel::INFO);

	~CLogger();

	CLogStream &Stream() { return _header._stream; }

private:
	CLogHeader _header;
	LogLevel _level;
};

#define LOG_TRACE if (CLoggerLogLevel() <= LogLevel::TRACE) \
	CLogger(__FILE__, __LINE__, LogLevel::TRACE).Stream()

#define LOG_DEBUG if (CLoggerLogLevel() <= LogLevel::DEBUG) \
	CLogger(__FILE__, __LINE__, LogLevel::DEBUG).Stream()

#define LOG_INFO if (CLoggerLogLevel() <=LogLevel::INFO) \
	CLogger(__FILE__, __LINE__).Stream()

#define LOG_WARN CLogger(__FILE__, __LINE__, LogLevel::WARN).Stream()

#define LOG_ERROR CLogger(__FILE__, __LINE__, LogLevel::ERR).Stream()

#define LOG_FATAL CLogger(__FILE__, __LINE__, LogLevel::FATAL).Stream()
// #define LOG_SYSERR CLogger(__FILE__, __LINE__, false).Stream()
// #define LOG_SYSFATAL CLogger(__FILE__, __LINE__, true).Stream()

#endif