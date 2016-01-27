#ifndef CINIT_SERVER_MODULE_H
#define  CINIT_SERVER_MODULE_H

#include <Windows.h>
#include <iostream>
#include <map>
#include "ILogDefines.h"
#include "ISocketSysterm.h"
#include "CVestibuleServer.h" 
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
	CLogHelper _logHelper;
	CVestibuleServer _server;
	CSocketSystermHepler _sockHelper;
	HANDLE _serverEvent;
	HANDLE _releaseEvent;
	OperateMode  _lastOpMode;
};
#endif