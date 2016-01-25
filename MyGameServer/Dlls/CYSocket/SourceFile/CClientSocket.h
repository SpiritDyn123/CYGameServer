#ifndef CCLIENT_SOCKET_H
#define CCLIENT_SOCKET_H

#include <Windows.h>
#include <iostream>
#include <map>
#include <queue>
#include <vector>
#include "ISocketSysterm.h"
#include "CConnectionChannel.h"
#include "InetAddress.h"
#include "CThreadQueue.h"
#include "COpensslEncrypter_Client.h"

class CClientSocket;

class ClientSocketThreadWork : public IThreadWork
{
public:
	ClientSocketThreadWork(CClientSocket *so) :
		_socket(so)
	{}

	virtual void DoTask(void *data = NULL);

private:
	CClientSocket *_socket;
};

class CSocketSystem;

class CClientSocket : public IClientSocket
					, public ISocket
					, public IEventSink
{
	friend class ClientSocketThreadWork;

public:
	CClientSocket(IClientSocketSink *pSink, CSocketSystem *socketSys);

	~CClientSocket();

public:
	virtual bool Start(std::string ip, WORD port);

	virtual void Close();

	virtual void Send(char *data, int nLen);

	virtual void OnEvent();

	void SwapQueMsg(std::queue<TcpMsgPtr> &queMsgs);

	void InsertMsg(TcpMsgPtr &msg);

	IClientSocketSink * GetLink() { return _sink; }

	bool IsRun() { return _running; }
	
	void OnClosed();

private:
	CSocketSystem *_socketSys;
	IClientSocketSink *_sink;
	SOCKET _clientSocket;
	HANDLE _iocpPort;
	InetAddress _addr;
	CThreadQueue _threadQue;
	COpensslEncrypter_Client _encrypter;
	bool _rsaPassed;

	HANDLE _mutex;
	bool _running;

	Buffer _readBuffer;
	ClientSocketThreadWork *_clientThreadWork;
	std::queue<TcpMsgPtr> _queueMsgs;
	ConnChannelPtr _conChannelPtr;

	HANDLE _closedEventInThread;
};
#endif