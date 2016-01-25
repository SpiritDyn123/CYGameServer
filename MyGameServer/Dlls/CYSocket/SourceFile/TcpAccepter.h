#ifndef TCP_ACCEPTER_H
#define TCP_ACCEPTER_H

#include <Windows.h>
#include "Buffer.h"
#include "SocketOps.h"
#include "ISocketSysterm.h"
#include <functional>
#include "InetAddress.h"

class TcpAccepter
{
public:
	typedef std::function<void(SOCKET, InetAddress)> NewConnectionCallBack;
	TcpAccepter(IServerSocket *serSocket, NewConnectionCallBack newConnectionCB);

	~TcpAccepter();

	SOCKET GetSocketId() const { return _socketId; }

	bool IsListen() const { return _listenning; }

	void Bind(WORD port);

	void Listen();

	InetAddress GetInetAddr() { return _inetAddr; }

	virtual void HandleRead(SOCKET, InetAddress);

private:;
	IServerSocket *_serSocket;
	SOCKET _socketId;
	HANDLE _mutex;
	InetAddress _inetAddr;

	NewConnectionCallBack _newConnectionCB;
	bool _listenning;
};
#endif