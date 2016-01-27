#ifndef ILOG_DEFINES_H
#define ILOG_DEFINES_H

#include "ILog.h"

extern ILog *g_log;

#define LOG_TRACE(szFormat, ...) \
	g_log->Log_Trace(CLogHelper::GenerateLogHead(__FILE__, __LINE__, LogLevel::TRACE).append(szFormat).c_str(), ##__VA_ARGS__)

#define LOG_DEBUG(szFormat, ...) \
	g_log->Log_Debug(CLogHelper::GenerateLogHead(__FILE__, __LINE__, LogLevel::DEBUG).append(szFormat).c_str(), ##__VA_ARGS__)

#define LOG_INFO(szFormat, ...) \
	g_log->Log_Info(CLogHelper::GenerateLogHead(__FILE__, __LINE__, LogLevel::INFO).append(szFormat).c_str(), ##__VA_ARGS__)

#define LOG_WARN(szFormat, ...) \
	g_log->Log_Warn(CLogHelper::GenerateLogHead(__FILE__, __LINE__, LogLevel::WARN).append(szFormat).c_str(), ##__VA_ARGS__)

#define LOG_ERROR(szFormat, ...) \
	g_log->Log_Error(CLogHelper::GenerateLogHead(__FILE__, __LINE__, LogLevel::ERR).append(szFormat).c_str(), ##__VA_ARGS__)

#define LOG_FATAL(szFormat, ...) \
	g_log->Log_Fatal(CLogHelper::GenerateLogHead(__FILE__, __LINE__, LogLevel::FATAL).append(szFormat).c_str(), ##__VA_ARGS__)

#endif