#ifndef CDY_LOG_H
#define CDY_LOG_H

#include <ILog.h>
#include "CLog.h"
#include "CLogger.h"

class CDyLog : public ILog
{
private:
	virtual bool Create(char *pLogFileName, int RolSize = OneGBytes);

	virtual void Release();

	virtual void Log_Trace(LPCSTR szFormat, ...);

	virtual void Log_Debug(LPCSTR szFormat, ...);

	virtual void Log_Info(LPCSTR szFormat, ...);

	virtual void Log_Warn(LPCSTR szFormat, ...);

	virtual void Log_Error(LPCSTR szFormat, ...);

	virtual void Log_Fatal(LPCSTR szFormat, ...);

private:
	CLog _logMgr;
};

#endif