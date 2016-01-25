#ifndef CCONNECTION_CHANNEL_H
#define CCONNECTION_CHANNEL_H

#include <Windows.h>
#include "Buffer.h"
#include "SocketOps.h"
#include "InetAddress.h"
#include <memory>

class CConnectionChannel;
//typedef FixedBuffer<MSG_BUFF_LEN + 1> ConnectionBuffer;
typedef std::shared_ptr<CConnectionChannel> ConnChannelPtr;

typedef struct : public OVERLAPPED
{
	ConnChannelPtr connPtr;
} IOCP_HANDLE_DATA, *PIOCP_HANDLE_DATA;

typedef struct : public OVERLAPPED
{
	WSABUF _buff;
	BYTE  _state; // 0读1写
	//定义为socket设置的最大读数据长度，确保一次性能读完数据
	Buffer _writeBuffer;
} IOCP_DATA, *PIOCP_DATA;

struct TCP_MSG
{
	enum TCP_MSG_STATE
	{
		Accepted,//serversocket为新连接，clientsocke为connected
		Closed,
		ReadBuff,
		SendBuff
	};

	DWORD _connId;
	TCP_MSG_STATE _state;
	Buffer _buff;
};

typedef std::shared_ptr<TCP_MSG> TcpMsgPtr;

class ISocket
{
public:
	virtual void InsertMsg(TcpMsgPtr &msg) = NULL;
};

class CConnectionChannel
{
public:
	CConnectionChannel(ISocket *serSocket, DWORD conId, SOCKET socketId, InetAddress inetAddress);

	~CConnectionChannel();

	SOCKET & GetSocketId() { return _socketId; }
	DWORD & GetConnId() { return _conId; }

	void Send(char *data, int nLen);

	void OnDisconnect();

	void OnError(DWORD errCode);

	void OnRecvCompelet(char *data, DWORD bytes);

	void OnSendCompelet(PIOCP_DATA iocpData, DWORD bytes);

	InetAddress &GetInetAddr() { return _inetAddress; }

private:;
	ISocket *_serSocket;
	DWORD _conId;
	SOCKET _socketId;
	HANDLE _mutex;
	InetAddress _inetAddress;

	Buffer _readBuffer;//读缓存
	Buffer _writeBuffer;//写缓存,由于iocp异步调用的关系，所以不能使用
};

#endif