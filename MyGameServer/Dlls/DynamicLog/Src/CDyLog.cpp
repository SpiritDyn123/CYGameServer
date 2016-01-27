#include "CDyLog.h"

bool CDyLog::Create(char *pLogFileName, int RolSize /*= OneGBytes*/)
{
	_logMgr.SetLogCustomHead(true);
	return _logMgr.Create(pLogFileName, RolSize);
}

void CDyLog::Release()
{
	_logMgr.Release();
}

void CDyLog::Log_Trace(LPCSTR szFormat, ...)
{
	if (CLoggerLogLevel() > LogLevel::TRACE)
		return;

	char szBuff[OneLogBuffSize] = { 0 };

	va_list args;
	va_start(args, szFormat);

	int nLen = _vsnprintf_s(szBuff, OneLogBuffSize, szFormat, args);
	if (nLen < 0)
	{
		LOG_ERROR << "写入一条log的长度大于 " << OneLogBuffSize;
		return;
	}

	LOG_TRACE << szBuff;
}

void CDyLog::Log_Debug(LPCSTR szFormat, ...)
{
	if (CLoggerLogLevel() > LogLevel::DEBUG)
		return;

	char szBuff[OneLogBuffSize] = { 0 };

	va_list args;
	va_start(args, szFormat);

	int nLen = _vsnprintf_s(szBuff, OneLogBuffSize, szFormat, args);
	if (nLen < 0)
	{
		LOG_ERROR << "写入一条log的长度大于 " << OneLogBuffSize;
		return;
	}

	LOG_DEBUG << szBuff;
}

void CDyLog::Log_Info(LPCSTR szFormat, ...)
{
	if (CLoggerLogLevel() > LogLevel::INFO)
		return;

	char szBuff[OneLogBuffSize] = { 0 };

	va_list args;
	va_start(args, szFormat);

	int nLen = _vsnprintf_s(szBuff, OneLogBuffSize, szFormat, args);
	if (nLen < 0)
	{
		LOG_ERROR << "写入一条log的长度大于 " << OneLogBuffSize;
		return;
	}

	LOG_INFO << szBuff;
}

void CDyLog::Log_Warn(LPCSTR szFormat, ...)
{
	char szBuff[OneLogBuffSize] = { 0 };

	va_list args;
	va_start(args, szFormat);

	int nLen = _vsnprintf_s(szBuff, OneLogBuffSize, szFormat, args);
	if (nLen < 0)
	{
		LOG_ERROR << "写入一条log的长度大于 " << OneLogBuffSize;
		return;
	}

	LOG_WARN << szBuff;
}

void CDyLog::Log_Error(LPCSTR szFormat, ...)
{
	char szBuff[OneLogBuffSize] = { 0 };

	va_list args;
	va_start(args, szFormat);

	int nLen = _vsnprintf_s(szBuff, OneLogBuffSize, szFormat, args);
	if (nLen < 0)
	{
		LOG_ERROR << "写入一条log的长度大于 " << OneLogBuffSize;
		return;
	}

	LOG_ERROR << szBuff;
}

void CDyLog::Log_Fatal(LPCSTR szFormat, ...)
{
	char szBuff[OneLogBuffSize] = { 0 };

	va_list args;
	va_start(args, szFormat);

	int nLen = _vsnprintf_s(szBuff, OneLogBuffSize, szFormat, args);
	if (nLen < 0)
	{
		LOG_ERROR << "写入一条log的长度大于 " << OneLogBuffSize;
		return;
	}

	LOG_FATAL << szBuff;
}