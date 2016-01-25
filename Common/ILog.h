#ifndef ILOG_H
#define ILOG_H

#include <Windows.h>

#define OneGBytes 1024 * 1024 * 1000

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
����һ��log�̣߳�ר�����ļ���дlog
**/
//////////////////////////////////////////////////////////////////////////
class ILog
{
public:
	virtual bool Create(char *pLogFileName, int RolSize = OneGBytes) = NULL;

	virtual void Release() = NULL;

// 	virtual void Log_Trace(...) = NULL;
// 
// 	virtual void Log_Debug(...) = NULL;
// 
// 	virtual void Log_Info(...) = NULL;
// 
// 	virtual void Log_Warn(...) = NULL;
// 
// 	virtual void Log_Error(...) = NULL;
// 
// 	virtual void Log_Fatal(...) = NULL;
};

class CLogHelper
{
public:
	typedef bool(*CreateLogFun)(ILog **pLog);
	CLogHelper() :
		_hDll(NULL),
		_log(NULL)
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
			_hDll = LoadLibrary("Log.dll");
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

// 	ISocketSysterm * operator ->()
// 	{
// 		return _socketSys;
// 	}

	ILog * GetLog()
	{
		return _log;
	}

private:
	HINSTANCE _hDll;
	ILog *_log;
};
#endif