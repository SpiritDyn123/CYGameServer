#ifndef CLOGGER_H
#define CLOGGER_H

#include <iostream>
#include "CLogStream.h"
#include "ILog.h"

//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/*
log�ļ���ʽ��������_ʱ��.log
log�����ݸ�ʽ:(LogLevel �ļ���:line)(ʱ��):�ַ���\n
�ļ�������СΪ1G��ʱ��Ϊÿ��0��
*/
/************************************************************************/
//////////////////////////////////////////////////////////////////////////

void defaultLogOutput(const char* msg, int len, LogLevel level = LogLevel::INFO);

void defaultLogFlush();

extern LogLevel g_logLevel;

class CLogger
{
public:

	class SourceFile
	{
	public:
		SourceFile(char *fileName)
		{
			std::string strFName(fileName);
			_fileName = fileName + strFName.find_last_of('\\') + 1;
		}


		char *_fileName;
		int _size;
	};

	class CLogHeader
	{
	public:
		CLogHeader(LogLevel level, const SourceFile &file, int line);

		const char * formatTime();

		CLogStream _stream;
	};

public:
	CLogger(SourceFile file, int line, LogLevel level = LogLevel::INFO);

	~CLogger();

	CLogStream &Stream() { return _header._stream; }

	typedef void(*OutputFunc)(const char* msg, int len, LogLevel level);
	typedef void(*FlushFunc)();

	static void SetLogLevel(LogLevel level);

	static void SetOutputFunc(OutputFunc);
	static void SetOutputFlushFunc(FlushFunc);

	static LogLevel logLevel() { return g_logLevel; }

private:
	CLogHeader _header;
	LogLevel _level;
};

#define LOG_TRACE if (CLogger::logLevel() <= LogLevel::TRACE) \
	CLogger(__FILE__, __LINE__, LogLevel::TRACE).Stream()

#define LOG_DEBUG if (CLogger::logLevel() <= LogLevel::DEBUG) \
	CLogger(__FILE__, __LINE__, CLogger::DEBUG).Stream()

#define LOG_INFO if (CLogger::logLevel() <=LogLevel::INFO) \
	CLogger(__FILE__, __LINE__).Stream()

#define LOG_WARN CLogger(__FILE__, __LINE__, LogLevel::WARN).Stream()

#define LOG_ERROR CLogger(__FILE__, __LINE__, LogLevel::ERR).Stream()

#define LOG_FATAL CLogger(__FILE__, __LINE__, LogLevel::FATAL).Stream()
// #define LOG_SYSERR CLogger(__FILE__, __LINE__, false).Stream()
// #define LOG_SYSFATAL CLogger(__FILE__, __LINE__, true).Stream()

#endif