#include "../stdafx.h"
#include "CSocketSystem.h"

CSocketSystem::CSocketSystem() :
_mainLogicThread(this),
_running(false)
{
	
}

CSocketSystem::~CSocketSystem()
{
	if (_running)
		Release();
}

ILogicThread *CSocketSystem::GetLogicThread()
{
	return static_cast<ILogicThread *>(&_mainLogicThread);
}

bool CSocketSystem::Create(std::string fileName)
{
	if (_running)
	{
		LOG_ERROR << "CSocketSystem 已经启动过";
		return false;
	}

	assert(_logMgr.Create(fileName.c_str()));
	assert(_mainLogicThread.Create());

	_running = true;

	return true;
}

void CSocketSystem::Release()
{
	if (!_running)
		return;

	_mainLogicThread.Release();
	_logMgr.Release();
	if (_serverSocket)
	{
		delete _serverSocket;
		_serverSocket = NULL;
	}

	for (int i = 0; i < (int)_vecAllClients.size(); i++)
	{
		delete _vecAllClients[i];
	}
	_vecAllClients.clear();
}

IServerSocket * CSocketSystem::CreateServerSocket(IServerSocketSink *pServerSocketSink)
{
	if (!_mainLogicThread.IsRunInLoop())
	{
		LOG_ERROR << "CreateServerSocket 必须在主逻辑线程中";
		return NULL;
	}

	if (!_running)
		return NULL;

	if (!_serverSocket)
		_serverSocket = new CServerSocket(pServerSocketSink, this);

	return _serverSocket;
}
IClientSocket * CSocketSystem::CreateClientSocket(IClientSocketSink *pClientSocketSink)
{
	if (!_mainLogicThread.IsRunInLoop())
	{
		LOG_ERROR << "CreateClientSocket 必须在主逻辑线程中";
		return NULL;
	}

	if (!_running)
		return NULL;

	CClientSocket *clientSocket = new CClientSocket(pClientSocketSink, this);
	_vecAllClients.push_back(clientSocket);
	return clientSocket;
}

CServerSocket * CSocketSystem::GetServerSocket() const
{
	return dynamic_cast<CServerSocket *>(_serverSocket);
}

std::vector<CClientSocket *> CSocketSystem::GetAllClientSockets() const
{
	return _vecAllClients;
}
