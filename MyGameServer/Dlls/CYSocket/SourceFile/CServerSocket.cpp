#include "../stdafx.h"
#include "CServerSocket.h"
#include <functional>
#include "CSocketSystem.h"
#include <memory>

void ServerSocketAcceptThreadWork::DoTask(void *data)
{
	SOCKET acceptSocket;
	int acceptAddrLen = 0;
	SOCKADDR_IN acceptAddr;
	while (_socket->_running)
	{
		acceptAddrLen = sizeof SOCKADDR_IN;
		acceptSocket = WSAAccept(_socket->_accepter.GetSocketId(), (SOCKADDR *)&acceptAddr, &acceptAddrLen, NULL, 10);
		if (acceptSocket == INVALID_SOCKET)
		{
			DWORD nLastError = GetLastError();
			if (nLastError != WSAEWOULDBLOCK)
			{
				LOG_ERROR << "Accept Error=" << nLastError;
			}
			continue;
		}

		if (sizeof SOCKADDR_IN != acceptAddrLen)
		{
			LOG_ERROR << "Accept Address Error";
			closesocket(acceptSocket);
			continue;
		}

		_socket->_accepter.HandleRead(acceptSocket, InetAddress(acceptAddr));
	}

	return;
}

void ServerSocketThreadWork::DoTask(void *data)
{
	PIOCP_HANDLE_DATA iocpHandleData = NULL;
	PIOCP_DATA iocpData;
	DWORD byteTransed;
	DWORD byteRecved;
	//DWORD byteSended;
	DWORD flags;
	while (_socket->_running)
	{
		iocpHandleData = NULL;
		iocpData = NULL;

		if (!GetQueuedCompletionStatus(_socket->_iocpPort, &byteTransed, (PULONG_PTR)&iocpHandleData, (LPOVERLAPPED *)&iocpData, 10))
		{
			if (iocpData == NULL || iocpHandleData == NULL)
				continue;
			else
			{
				if (byteTransed == 0)
				{
				CONNECT_DISCONNECT:
					iocpHandleData->connPtr->OnDisconnect();
					_socket->RemoveConnectionChannel(iocpHandleData->connPtr);
					delete iocpHandleData;
					delete iocpData;
					continue;
				}
				else
				{
					DWORD nLastErrorCode = GetLastError();
					LOG_ERROR << "IOCP GetQueued ErrorCode=" << nLastErrorCode;
					iocpHandleData->connPtr->OnError(nLastErrorCode);
					_socket->RemoveConnectionChannel(iocpHandleData->connPtr);
					delete iocpHandleData;
					delete iocpData;
					continue;
				}
			}
		}

		if (iocpData == NULL || iocpHandleData == NULL)
			continue;

		if (byteTransed == 0)
			goto CONNECT_DISCONNECT;

		if (iocpData->_state == 1)//写完成
		{
			iocpHandleData->connPtr->OnSendCompelet(iocpData, byteTransed);
		}
		else//读完成
		{
			iocpHandleData->connPtr->OnRecvCompelet(iocpData->_writeBuffer.beginWrite(), byteTransed);
			iocpData->_buff.len = iocpData->_writeBuffer.writableBytes();
			iocpData->_buff.buf = iocpData->_writeBuffer.beginWrite();

			flags = 0;
			if (WSARecv(iocpHandleData->connPtr->GetSocketId(), &iocpData->_buff, 1, &byteRecved, &flags, (LPOVERLAPPED)iocpData, NULL) == SOCKET_ERROR)
			{
				DWORD lastErrorCode = GetLastError();
				if (lastErrorCode != ERROR_IO_PENDING)
				{
					LOG_ERROR << "IOCP GetQueued WSARecv ErrorCode=" << lastErrorCode;
					iocpHandleData->connPtr->OnError(lastErrorCode);
					_socket->RemoveConnectionChannel(iocpHandleData->connPtr);
					delete iocpHandleData;
					delete iocpData;
				}
			}
		}
	}
}

CServerSocket::CServerSocket(IServerSocketSink *pSink, CSocketSystem *socketSys) :
_socketSys(socketSys),
_sink(pSink),
_nextConIndex(0),
_accepter(this, std::bind(&CServerSocket::OnNewConnection, this, std::placeholders::_1, std::placeholders::_2)),
_mutex(NULL),
_running(false),
_maxConnNum(DEFAUT_MAX_CONN)
{
	assert(_socketSys);
	_vecSocketWorkThread.clear();
}

CServerSocket::~CServerSocket()
{

}

bool CServerSocket::Start(WORD port, int MaxConnNum)
{
	LogicThreadMain *mainThread = dynamic_cast<LogicThreadMain *>(_socketSys->GetLogicThread());
	if (!mainThread || !mainThread->IsRunInLoop())
	{
		LOG_ERROR << "Start 必须在主逻辑线程中";
		return false;
	}

	_iocpPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (_iocpPort == NULL)
	{
		LOG_ERROR << "CreateIoCompletionPort error";
		return false;
	}

	_accepter.Bind(port);

	_mutex = CreateMutex(NULL, FALSE, NULL);
	if (_mutex == NULL)
	{
		LOG_ERROR << "ServerSocket CreateMutex Error";
		return false;
	}

	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);

	if (!_threadQue.Create(mySysInfo.dwNumberOfProcessors * 2 + 2))
	{
		return false;
	}

	_running = true;
	assert(!_accepter.IsListen());
	_accepter.Listen();

	_maxConnNum = MaxConnNum > 0 ? MaxConnNum : _maxConnNum;

	_socketAcceptWorkThread = new ServerSocketAcceptThreadWork(this);
	_threadQue.InsertThreadWorkObj(_socketAcceptWorkThread);
	for (int i = 0; i < (int)mySysInfo.dwNumberOfProcessors * 2;i++)
	{
		_vecSocketWorkThread.push_back(new ServerSocketThreadWork(this));
		_threadQue.InsertThreadWorkObj(_vecSocketWorkThread[i]);
	}

	return true;
}

void CServerSocket::Close()
{
	LogicThreadMain *mainThread = dynamic_cast<LogicThreadMain *>(_socketSys->GetLogicThread());
	if (!mainThread || !mainThread->IsRunInLoop())
	{
		LOG_ERROR << "Close 必须在主逻辑线程中";
		return;
	}

	_running = false;
	_threadQue.Release();

	delete _socketAcceptWorkThread;
	for (int i = 0; i < (int)_vecSocketWorkThread.size(); i++)
	{
		delete _vecSocketWorkThread[i];
	}
	_vecSocketWorkThread.clear();

	//清理所有消息
	WaitForSingleObject(_mutex, INFINITE);
	while (!_queueMsgs.empty())
	{
		_queueMsgs.pop();
	}
	ReleaseMutex(_mutex);

	CloseHandle(_iocpPort);
	CloseHandle(_mutex);
	closesocket(_accepter.GetSocketId());
}

void CServerSocket::Send(DWORD conId, char *data, int nLen)
{
	if (data == NULL || nLen <= 0)
		return;

	WaitForSingleObject(_mutex, INFINITE);
	std::map<DWORD, ConnChannelPtr>::iterator iter = _mapAllConnections.find(conId);
	if (iter != _mapAllConnections.end())
	{
		std::weak_ptr<CConnectionChannel> iocpHandleConnWtr = iter->second;
		if (iocpHandleConnWtr.lock())
		{
			Buffer tempBuffer;
			tempBuffer.write((char *)&nLen, sizeof(int));
			tempBuffer.write(data, nLen);

			Buffer sndBuff(sizeof(int) + nLen + 15);
			int encryOutLen = 0;
			//加密
			{
				if (!_encrypter.Encrypter(tempBuffer.beginRead(), sndBuff.beginWrite(), tempBuffer.readableBytes(), encryOutLen))
				{
					ReleaseMutex(_mutex);
					LOG_ERROR << "加密失败";
					return;
				}
				iocpHandleConnWtr.lock()->Send(sndBuff.beginWrite(), encryOutLen);
			}			
		}
		else
		{
			LOG_ERROR << "mapConnections里存在空指针";
			_mapAllConnections.erase(conId);
		}
	}
	ReleaseMutex(_mutex);
}

void CServerSocket::CloseSocket(DWORD conId)
{
	LogicThreadMain *mainThread = dynamic_cast<LogicThreadMain *>(_socketSys->GetLogicThread());
	if (!mainThread || !mainThread->IsRunInLoop())
	{
		LOG_ERROR << "Start 必须在主逻辑线程中";
		return;
	}

	WaitForSingleObject(_mutex, INFINITE);
	std::map<DWORD, ConnChannelPtr>::iterator iter = _mapAllConnections.find(conId);
	if (iter != _mapAllConnections.end())
	{
		std::weak_ptr<CConnectionChannel> iocpHandleConnWtr = iter->second;
		ConnChannelPtr conPtr = iocpHandleConnWtr.lock();
		if (conPtr)
		{
			closesocket(conPtr->GetSocketId());
		}
	}
	ReleaseMutex(_mutex);
}

bool CServerSocket::GetClientInfo(DWORD conId, char *szIp, WORD &port)
{
	if (szIp == NULL)
		return false;

	std::weak_ptr<CConnectionChannel> connWtr;
	WaitForSingleObject(_mutex, INFINITE);
	std::map<DWORD, ConnChannelPtr>::iterator iter = _mapAllConnections.find(conId);
	if (iter == _mapAllConnections.end())
	{
		ReleaseMutex(_mutex);
		return false;
	}
	connWtr = iter->second;
	ReleaseMutex(_mutex);

	ConnChannelPtr connPtr = connWtr.lock();
	if (connPtr)
	{
		InetAddress &addr = connPtr->GetInetAddr();
		port = addr.GetPort();
		std::string ipStr = addr.GetIp();
		memcpy(szIp, ipStr.c_str(), ipStr.length());
		return true;
	}
	else
	{
		return false;
	}
}

void CServerSocket::InsertMsg(TcpMsgPtr &msg)
{
	if (!msg)
		return;

	if (!_running)
	{
		return;
	}

	if (msg->_state == TCP_MSG::ReadBuff)
	{
		//解密
		{
			int nLen = msg->_buff.readInt();
			msg->_buff.retrieve(sizeof(int));
			Buffer revBuff(nLen);
			int decryOutLen = 0;
			if (!_encrypter.Decrypter(msg->_buff.beginRead(), revBuff.beginWrite(), nLen, decryOutLen))
			{
				LOG_ERROR << "解密失败";

				WaitForSingleObject(_mutex, INFINITE);
				std::map<DWORD, ConnChannelPtr>::iterator iter = _mapAllConnections.find(msg->_connId);
				assert(iter != _mapAllConnections.end());
				std::weak_ptr<CConnectionChannel> connChannel = iter->second;
				assert(connChannel.lock());
				ReleaseMutex(_mutex);
				
				closesocket(connChannel.lock()->GetSocketId());
				return;
			}

			int nMsgLen = revBuff.readInt();
			if (nMsgLen < 0 || nMsgLen > decryOutLen)
			{
				LOG_ERROR << "解密后的消息长度有问题，可能解密失败";
				return;
			}

			msg->_buff.retrieve(nLen);
			
			msg->_buff.write(revBuff.beginWrite(), decryOutLen);
		}
	}

	WaitForSingleObject(_mutex, INFINITE);
	_queueMsgs.push(msg);
	ReleaseMutex(_mutex);
}

void CServerSocket::SwapQueMsg(std::queue<TcpMsgPtr> &queMsgs)
{
	WaitForSingleObject(_mutex, INFINITE);
	_queueMsgs.swap(queMsgs);
	ReleaseMutex(_mutex);
}

void CServerSocket::InsertConnectionChannel(ConnChannelPtr &_ConnChannel)
{
	if (!_ConnChannel)
		return;

	WaitForSingleObject(_mutex, INFINITE);
	assert(_mapAllConnections.find(_ConnChannel->GetConnId()) == _mapAllConnections.end());
	_mapAllConnections.insert(std::pair<DWORD,  ConnChannelPtr>(_ConnChannel->GetConnId(), _ConnChannel));
	ReleaseMutex(_mutex);
}

void CServerSocket::RemoveConnectionChannel(ConnChannelPtr &_ConnChannel)
{
	if (!_ConnChannel)
		return;

	WaitForSingleObject(_mutex, INFINITE);
	assert(_mapAllConnections.find(_ConnChannel->GetConnId()) != _mapAllConnections.end());
	_mapAllConnections.erase(_mapAllConnections.find(_ConnChannel->GetConnId()));
	ReleaseMutex(_mutex);
}

void CServerSocket::SendRsaSeed(ConnChannelPtr &_ConnChannel)
{
	if (!_ConnChannel)
		return;

	WaitForSingleObject(_mutex, INFINITE);
	int nRsaKeyLen = 0;
	int nSeedLen = 0;
	char *rsaKeyData = (char *)_encrypter.GetPublicRsaKey(nRsaKeyLen);
	char *seedData = (char *)_encrypter.GetPublicAesSeed(nSeedLen);
	if (rsaKeyData == NULL || seedData == NULL)
	{
		ReleaseMutex(_mutex);
		return;
	}

	Buffer sndBuff;
	sndBuff.write(rsaKeyData, nRsaKeyLen);
	sndBuff.write(seedData, nSeedLen);
	_ConnChannel->Send(sndBuff.beginRead(), sndBuff.readableBytes());

	ReleaseMutex(_mutex);
}

void CServerSocket::OnNewConnection(SOCKET socketId, InetAddress inetAddr)
{
	//判断是否达到最大连接数
	int connNum = 0;
	WaitForSingleObject(_mutex, INFINITE);
	connNum = _mapAllConnections.size();
	ReleaseMutex(_mutex);

	if (connNum >= _maxConnNum)
	{
		closesocket(socketId);
		return;
	}

	int nConnId = _nextConIndex++;
	ConnChannelPtr connChannel(new CConnectionChannel(this, nConnId, socketId, inetAddr));
	if (!connChannel)
	{
		LOG_ERROR << "IOCP Accept new CConnectionChannel Error=" << GetLastError();
		closesocket(socketId);
		_nextConIndex--;
		return;
	}

	PIOCP_HANDLE_DATA iocpHandleData = new IOCP_HANDLE_DATA;
	iocpHandleData->connPtr = connChannel;
	if (!CreateIoCompletionPort((HANDLE)socketId, _iocpPort, (ULONG_PTR)iocpHandleData, 0))
	{
		LOG_ERROR << "IOCP Accept CreateIoCompletionPort Error=" << GetLastError();
		closesocket(socketId);
		_nextConIndex--;
		delete iocpHandleData;
		return;
	}

	InsertConnectionChannel(connChannel);
	SendRsaSeed(connChannel);//接受连接后发送rsa和seed

	TcpMsgPtr msg(new TCP_MSG);
	msg->_connId = nConnId;
	msg->_state = TCP_MSG::Accepted;
	InsertMsg(msg);

	PIOCP_DATA iocpData;
	DWORD byteRecved;
	DWORD flags;

	iocpData = new IOCP_DATA;
	memset(iocpData, 0, sizeof(OVERLAPPED));
	iocpData->_state = 0;
	//iocpData->_writeBuffer.Resize(SOCKET_BUFF_LEN);//不用SOCKET_BUFF_LEN,可以分次读取
	iocpData->_buff.len = iocpData->_writeBuffer.writableBytes();
	iocpData->_buff.buf = iocpData->_writeBuffer.beginWrite();

	flags = 0;
	if (WSARecv(socketId, &iocpData->_buff, 1, &byteRecved, &flags, (LPOVERLAPPED)iocpData, NULL) == SOCKET_ERROR)
	{
		DWORD lastErrorCode = GetLastError();
		if (lastErrorCode != ERROR_IO_PENDING)
		{
			LOG_ERROR << "IOCP Accept WSARecv ErrorCode=" << lastErrorCode;
			connChannel->OnError(lastErrorCode);
			RemoveConnectionChannel(connChannel);
			delete iocpData;
			delete iocpHandleData;
		}
	}

	LOG_INFO << "new connection socketId=" << socketId;
}