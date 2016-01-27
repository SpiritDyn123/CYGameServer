#include "stdafx.h"
#include "CInitServerModule.h"

ILog * g_log = NULL;

CInitServerModule::CInitServerModule() :
_lastOpMode(omNone),
_serverEvent(CreateEvent(NULL, FALSE, FALSE, NULL)),
_releaseEvent(CreateEvent(NULL, FALSE, FALSE, NULL))
{
	assert(_serverEvent);
	assert(_releaseEvent);
}

CInitServerModule::~CInitServerModule()
{
	CloseHandle(_serverEvent);
	_serverEvent = NULL;

	CloseHandle(_releaseEvent);
	_releaseEvent = NULL;
}

bool CInitServerModule::Create()
{
	_logHelper.Create();
	g_log = _logHelper.GetLog();
	assert(g_log);
	if (!g_log->Create("./Logs/DBVestibuleServer"))
	{
		return false;
	}

	_sockHelper.Create();

	if (!_sockHelper->Create("./Logs/DBVSSocket"))
	{
		LOG_ERROR("_sockHelper Create Error");
		return false;
	}

	ILogicThread *logicThread = _sockHelper->GetLogicThread();
	if (!logicThread)
	{
		LOG_ERROR("_sockHelper GetLogicThread Error");
		_sockHelper->Release();
		return false;
	}

	logicThread->AddEvent(_serverEvent, this);
	SetEventMode(omStartServer);

	return true;
}

void CInitServerModule::Release()
{
	SetEventMode(omCloseServer);
	LOG_INFO("服务器关闭中...");

	WaitForSingleObject(_releaseEvent, INFINITE);

	ILogicThread *logThread = _sockHelper->GetLogicThread();
	if (logThread)
	{
		logThread->RemoveEvent(_serverEvent);
	}

	_sockHelper->Release();

	_sockHelper.Release();

	g_log->Release();

	_logHelper.Release();
}

void CInitServerModule::SetEventMode(OperateMode mode)
{
	_lastOpMode = mode;
	SetEvent(_serverEvent);
}

void CInitServerModule::OnEvent()
{
	if (_lastOpMode == omStartServer)
	{
		_server.StartServer(_sockHelper.GetSocketSys());
	}
	else if (_lastOpMode == omCloseServer)
	{
		_server.CloseServer(_sockHelper.GetSocketSys());
		SetEvent(_releaseEvent);
	}
}
