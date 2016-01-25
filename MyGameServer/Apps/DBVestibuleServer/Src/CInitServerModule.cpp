#include "stdafx.h"
#include "CInitServerModule.h"
#include "CLogger.h"

CInitServerModule::CInitServerModule() :
_lastOpMode(omNone),
_serverEvent(CreateEvent(NULL, FALSE, FALSE, NULL))
{
	assert(_serverEvent);
}

CInitServerModule::~CInitServerModule()
{
	CloseHandle(_serverEvent);
	_serverEvent = NULL;
}

bool CInitServerModule::Create()
{
	if (!_logMgr.Create("./Logs/DbVestibuleServer"))
	{
		return false;
	}

	_sockHelper.Create();

	if (!_sockHelper->Create("./Logs/DbVSSocket"))
	{
		LOG_ERROR << "_sockHelper Create Error";
		return false;
	}

	ILogicThread *logicThread = _sockHelper->GetLogicThread();
	if (!logicThread)
	{
		LOG_ERROR << "_sockHelper GetLogicThread Error";
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
	LOG_INFO << "服务器关闭中...";
	Sleep(3000);

	ILogicThread *logThread = _sockHelper->GetLogicThread();
	if (logThread)
	{
		logThread->RemoveEvent(_serverEvent);
	}

	_sockHelper.Release();
	_logMgr.Release();
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
	}
}