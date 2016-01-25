#ifndef CINIT_SERVER_MODULE_H
#define  CINIT_SERVER_MODULE_H

#include <Windows.h>
#include <iostream>
#include <map>

#include "ISocketSysterm.h"
#include "CVestibuleServer.h" 
#include "CLog.h"
#include "CLogger.h"
#include "CIniFile.h"
class CInitServerModule : public IEventSink
{	
public:
	enum OperateMode
	{
		omNone,
		omStartServer,
		omCloseServer,
	};

	CInitServerModule();

	~CInitServerModule();

	bool Create();

	void Release();

	void SetEventMode(OperateMode mode);

	virtual void OnEvent();

private:
	CLog _logMgr;
	CVestibuleServer _server;
	CSocketSystermHepler _sockHelper;
	HANDLE _serverEvent;
	OperateMode  _lastOpMode;
};
#endif