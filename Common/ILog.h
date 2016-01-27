#ifndef ILOG_H
#define ILOG_H

#include <Windows.h>
#include <iostream>
#include <time.h>

#define OneGBytes 1024 * 1024 * 1000
#define OneLogBuffSize 1024

enum LogLevel
{
	TRACE,
	DEBUG,
	INFO,
	WARN,
	ERR,
	FATAL,
	NUM_LOG_LEVELS,
};

//////////////////////////////////////////////////////////////////////////
/**
创建一个log线程，专门往文件里写log
**/
//////////////////////////////////////////////////////////////////////////
class ILog
{
public:
	virtual bool Create(char *pLogFileName, int RolSize = OneGBytes) = NULL;

	virtual void Release() = NULL;

	virtual void Log_Trace(LPCSTR szFormat, ...) = NULL;
 
	virtual void Log_Debug(LPCSTR szFormat, ...) = NULL;

	virtual void Log_Info(LPCSTR szFormat, ...) = NULL;

	virtual void Log_Warn(LPCSTR szFormat, ...) = NULL;

	virtual void Log_Error(LPCSTR szFormat, ...) = NULL;

	virtual void Log_Fatal(LPCSTR szFormat, ...) = NULL;
};

class CLogHelper
{
public:
	typedef void(*CreateLogFun)(ILog **pLog);
	CLogHelper() :
		_hDll(NULL)
	{

	}

	~CLogHelper()
	{
		Release();
	}

	void Create()
	{
		Release();
		try
		{
			_hDll = LoadLibrary("DynamicLog.dll");
			if (_hDll == NULL)
			{
				throw "Can't load ChunYanNetwork.dll";
			}

			CreateLogFun proc = (CreateLogFun)GetProcAddress(_hDll, "CreateLog");
			if (proc == NULL)
				throw "Can't GetProcAddress('CreateLogFun')";

			proc(&_log);
		}
		catch (LPCSTR errMgs)
		{
			throw errMgs;
		}
	}

	void Release()
	{
		if (_hDll != NULL)
		{
			::FreeLibrary(_hDll);
			_hDll = NULL;
		}
	}

	ILog * operator ->()
	{
		return _log;
	}

	ILog * GetLog()
	{
		return _log;
	}

public:
	static std::string GenerateFormatFileName(char *file)
	{
		std::string strFName(file);
		file = file + strFName.find_last_of('\\') + 1;
		return std::string(file);
	}

	static std::string GenerateLogHead(char *file, int line, LogLevel level)
	{
		static const char *g_logLevelName[] =
		{
			"TRACE",
			"DEBUG",
			"INFO",
			"WARN",
			"ERR",
			"FATAL",
		};

		std::string fFileName = GenerateFormatFileName(file);

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

		char szBuff[OneLogBuffSize] = { 0 };
		//(LogLevel 文件名:line)(时间):
		sprintf_s(szBuff, OneLogBuffSize, "(%s %s:%d)(%s):", g_logLevelName[level], fFileName.c_str(), line, timeStr);
		return std::string(szBuff);
	}

private:
	HINSTANCE _hDll;
	ILog *_log;
};
#endif