#ifndef CSOCKET_SYSTEM_H
#define  CSOCKET_SYSTEM_H

#include <Windows.h>
#include <iostream>
#include "ISocketSysterm.h"
#include "LogicThreadMain.h"
#include "CServerSocket.h"
#include "CClientSocket.h"

class CSocketSystem : public ISocketSysterm
{
public:
	CSocketSystem();
	~CSocketSystem();
public:
// 	static CSocketSystem *GetInstance()
// 	{
// 		static CSocketSystem t;
// 		return &t;
// 	}

	virtual bool Create(std::string fileName);

	virtual void Release();

	virtual ILogicThread *GetLogicThread();

	virtual IServerSocket * CreateServerSocket(IServerSocketSink *pServerSocketSink);

	virtual IClientSocket * CreateClientSocket(IClientSocketSink *pClientSocketSink);
	
	CServerSocket * GetServerSocket() const;

	std::vector<CClientSocket *> GetAllClientSockets() const;

private:
	CLog _logMgr;
	IServerSocket *_serverSocket;//最好dll只到处一个gamesocket和多个clientsocket
	std::vector<CClientSocket *> _vecAllClients;
	LogicThreadMain _mainLogicThread;

	bool _running;
};

//#define g_socketSys CSocketSystem::GetInstance()
#endif