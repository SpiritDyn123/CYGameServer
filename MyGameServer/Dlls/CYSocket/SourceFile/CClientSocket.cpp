#include "../stdafx.h"
#include "CClientSocket.h"
#include <functional>
#include "CSocketSystem.h"

void ClientSocketThreadWork::DoTask(void *data)
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
					_socket->OnClosed();
					delete iocpHandleData;
					delete iocpData;
					continue;
				}
				else
				{
					DWORD nLastErrorCode = GetLastError();
					LOG_ERROR << "IOCP GetQueued ErrorCode=" << nLastErrorCode;
					iocpHandleData->connPtr->OnError(nLastErrorCode);
					_socket->OnClosed();
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
					_socket->OnClosed();
					delete iocpHandleData;
					delete iocpData;
				}
			}
		}
	}
}

CClientSocket::CClientSocket(IClientSocketSink *pSink, CSocketSystem *socketSys) :
_socketSys(socketSys),
_sink(pSink),
_clientSocket(INVALID_SOCKET),
_mutex(NULL),
_rsaPassed(false),
_running(false)
{
	_closedEventInThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(_closedEventInThread);
}

CClientSocket::~CClientSocket()
{
	CloseHandle(_closedEventInThread);
	_closedEventInThread = NULL;
}

bool CClientSocket::Start(std::string ip, WORD port)
{
	LogicThreadMain *mainThread = dynamic_cast<LogicThreadMain *>(_socketSys->GetLogicThread());
	if (!mainThread || !mainThread->IsRunInLoop())
	{
		LOG_ERROR << "Start 必须在主逻辑线程中";
		return false;
	}

	if (_running)
		return false;

	_iocpPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (_iocpPort == NULL)
	{
		LOG_ERROR << "ClientSocket Start CreateIoCompletionPort error";
		return false;
	}

	_clientSocket = CreateTcpSocket();
	if (_clientSocket == INVALID_SOCKET)
	{
		LOG_ERROR << "ClientSocket CreateTcpSocket Error";
		return false;
	}

	_mutex = CreateMutex(NULL, FALSE, NULL);
	if (_mutex == NULL)
	{
		LOG_ERROR << "ClientSocket CreateMutex Error";
		return false;
	}

	PIOCP_HANDLE_DATA pHandleData = new IOCP_HANDLE_DATA;
	ConnChannelPtr conPtr(new CConnectionChannel(this, 0, _clientSocket, _addr));
	pHandleData->connPtr = conPtr;

	if (!CreateIoCompletionPort((HANDLE)_clientSocket, _iocpPort, (ULONG_PTR)pHandleData, 0))
	{
		LOG_ERROR << "ClientSocket Start bind IOCP CreateIoCompletionPort Error";
		delete pHandleData;
		return false;
	}

	_addr.Init(ip, port);
	SOCKADDR_IN *addr = _addr.GetSockAddr();
	if (connect(_clientSocket, (SOCKADDR *)addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		if (GetLastError() != WSAEWOULDBLOCK)
		{
			LOG_ERROR << "ClientSocket Connect Error" << GetLastError();
			delete pHandleData;
			return false;
		}	
	}

	//投递接收
	PIOCP_DATA iocpData = new IOCP_DATA;
	memset(iocpData, 0, sizeof(OVERLAPPED));
	iocpData->_state = 0;
	iocpData->_writeBuffer.Resize(SOCKET_BUFF_LEN);//确保缓存==socket最大读缓存
	iocpData->_buff.len = iocpData->_writeBuffer.writableBytes();
	iocpData->_buff.buf = iocpData->_writeBuffer.beginWrite();

	DWORD byteRecved;
	DWORD flags = 0;
	_running = true;
	if (WSARecv(_clientSocket, &iocpData->_buff, 1, &byteRecved, &flags, (LPOVERLAPPED)iocpData, NULL) == SOCKET_ERROR)
	{
		DWORD lastErrorCode = GetLastError();
		if (lastErrorCode != ERROR_IO_PENDING)
		{
			LOG_ERROR << "IOCP GetQueued WSARecv ErrorCode=" << lastErrorCode;
			TcpMsgPtr msg(new TCP_MSG);
			msg->_state = TCP_MSG::Closed;
			InsertMsg(msg);
			delete pHandleData;
			delete iocpData;
			_running = false;
			return false;
		}
		else
		{
			_conChannelPtr = conPtr;
			TcpMsgPtr msg(new TCP_MSG);
			msg->_state = TCP_MSG::Accepted;
			InsertMsg(msg);
		}
	}

	if (!_threadQue.Create(1))
	{
		_conChannelPtr.reset();
		delete pHandleData;
		delete iocpData;
		_running = false;
		return false;
	}

	//mainThread->AddEvent(_closedEventInThread, this);
	_clientThreadWork = new ClientSocketThreadWork(this);
	_threadQue.InsertThreadWorkObj(_clientThreadWork);

	return true;
}

void CClientSocket::Close()
{
	LogicThreadMain *mainThread = dynamic_cast<LogicThreadMain *>(_socketSys->GetLogicThread());
	if (!mainThread || !mainThread->IsRunInLoop())
	{
		LOG_ERROR << "Close 必须在主逻辑线程中";
		return;
	}

	if (!_running)
		return;

	//mainThread->RemoveEvent(_closedEventInThread);

	_running = false;
	_rsaPassed = false;
	_threadQue.Release();
	delete _clientThreadWork;
	_conChannelPtr.reset();

	//清理所有消息
	// 	WaitForSingleObject(_mutex, INFINITE);
	// 	while (!_queueMsgs.empty())
	// 	{
	// 		TCP_MSG *msg = _queueMsgs.front();
	// 		if (msg)
	// 			delete msg;
	// 		_queueMsgs.pop();
	// 	}
	// 	ReleaseMutex(_mutex);

	CloseHandle(_iocpPort);
	CloseHandle(_mutex);
	closesocket(_clientSocket);
}

void CClientSocket::Send(char *data, int nLen)
{
	if (data == NULL || nLen <= 0)
		return;

	if (!_rsaPassed)
		return;

	if (nLen > MSG_BUFF_LEN)
	{
		LOG_ERROR << "发送消息长度过大";
		return;
	}

	if (_conChannelPtr)
	{
		Buffer sndBuff(sizeof(int)+nLen + 15);
		//加密
		{
			Buffer tempBuffer;
			tempBuffer.write((char *)&nLen, sizeof(int));
			tempBuffer.write(data, nLen);

		
			int encryOutLen = 0;
			if (!_encrypter.Encrypter(tempBuffer.beginRead(), sndBuff.beginWrite(), tempBuffer.readableBytes(), encryOutLen))
			{
				LOG_ERROR << "加密失败";
				return;
			}
		
			nLen = encryOutLen;
			data = sndBuff.beginWrite();
		}

		_conChannelPtr->Send(data, nLen);
	}
}

void CClientSocket::OnEvent()
{
	
}

void CClientSocket::InsertMsg(TcpMsgPtr &msg)
{
	if (!msg)
		return;

	if (!_running)
	{
		return;
	}

	if (msg->_state == TCP_MSG::ReadBuff)
	{
		if (_rsaPassed)
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
		else
		{
			int nLen = msg->_buff.readInt();
			msg->_buff.retrieve(sizeof(int));
			if (!_encrypter.SetAndGenerateRsaKey((unsigned char *)msg->_buff.beginRead(), nLen))
			{
				LOG_ERROR << "服务器加密解密seed接受失败";
				closesocket(_clientSocket);
				return;
			}
			_rsaPassed = true;
			return;
		}
	}
	

	WaitForSingleObject(_mutex, INFINITE);
	_queueMsgs.push(msg);
	ReleaseMutex(_mutex);
}

void CClientSocket::SwapQueMsg(std::queue<TcpMsgPtr> &queMsgs)
{
	WaitForSingleObject(_mutex, INFINITE);
	_queueMsgs.swap(queMsgs);
	ReleaseMutex(_mutex);
}

void CClientSocket::OnClosed()
{
	//SetEvent(_closedEventInThread);
}
