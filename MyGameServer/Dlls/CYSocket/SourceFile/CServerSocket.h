#ifndef CSERVER_SOCKET_H
#define CSERVER_SOCKET_H

#include <Windows.h>
#include <iostream>
#include <map>
#include <queue>
#include <vector>
#include "ISocketSysterm.h"
#include "CConnectionChannel.h"
#include "InetAddress.h"
#include "CThreadQueue.h"
#include "COpensslEncrypter.h"
#include "TcpAccepter.h"

class CServerSocket;
class ServerSocketAcceptThreadWork : public IThreadWork
{
public:
	ServerSocketAcceptThreadWork(CServerSocket *so) :
		_socket(so)
	{}

	virtual void DoTask(void *data = NULL);

private:
	CServerSocket *_socket;
};

class ServerSocketThreadWork : public IThreadWork
{
public:
	ServerSocketThreadWork(CServerSocket *so) :
		_socket(so)
	{}

	virtual void DoTask(void *data = NULL);

private:
	CServerSocket *_socket;
};

class CSocketSystem;

class CServerSocket : public IServerSocket
					, public ISocket
{
	friend class ServerSocketAcceptThreadWork;
	friend class ServerSocketThreadWork;

public:
	CServerSocket(IServerSocketSink *pSink, CSocketSystem *socketSys);

	~CServerSocket();

public:
	virtual bool Start(WORD port, int MaxConnNum);

	virtual void Close();

	virtual void Send(DWORD conId, char *data, int nLen);

	virtual void CloseSocket(DWORD conId);

	virtual bool GetClientInfo(DWORD conId, char *szIp, WORD &port);

	virtual void InsertMsg(TcpMsgPtr &msg);

	void SwapQueMsg(std::queue<TcpMsgPtr> &queMsgs);

	IServerSocketSink * GetLink() { return _sink; }

	bool IsRun() { return _running; }

private:

	void InsertConnectionChannel(ConnChannelPtr &_ConnChannel);

	void RemoveConnectionChannel(ConnChannelPtr &_ConnChannel);

	void SendRsaSeed(ConnChannelPtr &_ConnChannel);

	void OnNewConnection(SOCKET socketId, InetAddress inetAddr);

private:
	IServerSocketSink *_sink;
	DWORD _nextConIndex;
	TcpAccepter _accepter;
	HANDLE _iocpPort;
	CThreadQueue _threadQue;
	COpensslEncrypter _encrypter;

	HANDLE _mutex;
	bool _running;

	ServerSocketAcceptThreadWork * _socketAcceptWorkThread;
	std::vector<ServerSocketThreadWork *> _vecSocketWorkThread;

	std::map<DWORD, ConnChannelPtr> _mapAllConnections;

	std::queue<TcpMsgPtr> _queueMsgs;

	CSocketSystem *_socketSys;

	int _maxConnNum;
};
#endif