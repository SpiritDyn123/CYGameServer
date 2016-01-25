#include "../stdafx.h"
#include "TcpAccepter.h"
#include "CServerSocket.h"

TcpAccepter::TcpAccepter(IServerSocket *serSocket, NewConnectionCallBack newConnectionCB) :
_serSocket(serSocket),
_newConnectionCB(newConnectionCB),
_socketId(CreateTcpSocket()),
_mutex(CreateMutex(NULL, FALSE,NULL)),
_listenning(false)
{
	assert(_socketId != INVALID_SOCKET);
	assert(_mutex);
}

TcpAccepter::~TcpAccepter()
{
	closesocket(_socketId);
	CloseHandle(_mutex);
	_listenning = false;
}

void TcpAccepter::Bind(WORD port)
{
	_inetAddr.Init(port);
	SOCKADDR_IN *addr = _inetAddr.GetSockAddr();
	if (bind(_socketId, (PSOCKADDR)addr, sizeof(SOCKADDR_IN)) < 0)
	{
		LOG_ERROR << "TcpAccepter bind Error";
		return;
	}
}

void TcpAccepter::Listen()
{
	if (listen(_socketId, TCP_SERVER_LISTEN_COUNT) < 0)
	{
		LOG_ERROR << "TcpAccepter listen Error";
		return;
	}
	_listenning = true;
}

void TcpAccepter::HandleRead(SOCKET s, InetAddress addr)
{
	if (_newConnectionCB)
	{
		_newConnectionCB(s, addr);
	}
	else
	{
		closesocket(s);
	}
}