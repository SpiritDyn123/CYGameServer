#include "../stdafx.h"
#include "CConnectionChannel.h"
#include "CServerSocket.h"
#include "CClientSocket.h"

CConnectionChannel::CConnectionChannel(ISocket *serSocket, DWORD conId, SOCKET socketId, InetAddress inetAddress) :
_serSocket(serSocket),
_inetAddress(inetAddress),
_conId(conId),
_socketId(socketId),
_mutex(CreateMutex(NULL, FALSE, NULL))
{
	assert(_mutex != NULL);
}

CConnectionChannel::~CConnectionChannel()
{
	if (_mutex)
	{
		CloseHandle(_mutex);
		_mutex = NULL;
	}
}

void CConnectionChannel::Send(char *data, int nLen)
{
	if (data == NULL || nLen <= 0)
		return;

	if (nLen > MSG_BUFF_LEN)
	{
		LOG_ERROR << "������Ϣ���ȹ���";
		return;
	}

	DWORD byteSended;
	DWORD flags = 0;
	PIOCP_DATA iocpData = new IOCP_DATA;
	iocpData->_state = 1;
	memset(iocpData, 0, sizeof(OVERLAPPED));

	iocpData->_writeBuffer.write((char *)&nLen, sizeof(int));
	iocpData->_writeBuffer.write(data, nLen);

	iocpData->_buff.len = iocpData->_writeBuffer.readableBytes();
	iocpData->_buff.buf = (char *)iocpData->_writeBuffer.beginRead();
	if(WSASend(GetSocketId(), &iocpData->_buff, 1, &byteSended, flags, (LPOVERLAPPED)iocpData, NULL) == SOCKET_ERROR)
	{
		DWORD lastErrorCode = GetLastError();
		if (lastErrorCode != ERROR_IO_PENDING)
		{
			LOG_ERROR << "IOCP WSASend ErrorCode=" << lastErrorCode;
			delete iocpData;
		}
	}
}

void CConnectionChannel::OnDisconnect()
{
	closesocket(_socketId);
	TcpMsgPtr msg(new TCP_MSG);
	msg->_connId = _conId;
	msg->_state = TCP_MSG::Closed;
	_serSocket->InsertMsg(msg);
}

void CConnectionChannel::OnError(DWORD errCode)
{
	OnDisconnect();
}

void CConnectionChannel::OnRecvCompelet(char *data, DWORD bytes)
{
	_readBuffer.write((char *)data, bytes);
	while (1)
	{
		if (_readBuffer.readableBytes() > sizeof(int))
		{
			int len = _readBuffer.readInt();
			if (len > 0 && len <= MSG_BUFF_LEN)//�涨һ����Ϣ���Ȳ��ܴ��ڶ���,��Ȼ���ܿͻ��˷��͸�ʽ�����Լ��������������
			{
				if (_readBuffer.readableBytes() >= len + (int)sizeof(int))
				{
					_readBuffer.retrieve(sizeof(int));

					TcpMsgPtr msg(new TCP_MSG);
					msg->_connId = _conId;
					msg->_state = TCP_MSG::ReadBuff;

					msg->_buff.Resize(len + sizeof(int));
					msg->_buff.write((char *)&len, sizeof(int));
					msg->_buff.write(_readBuffer.beginRead(), len);
					_readBuffer.retrieve(len);

					_serSocket->InsertMsg(msg);
				}
				else
				{
					break;
				}
			}
			else
			{
				//�����ϸ�ʽ��ô�������Ϣ���ȴ���socket_buff_len
				LOG_ERROR << "������Ϣ�������߸�ʽ����";
				closesocket(_socketId);
				break;
			}
		}
		else
		{
			break;
		}
	}
}

void CConnectionChannel::OnSendCompelet(PIOCP_DATA iocpData, DWORD bytes)
{
	int nRemainLen = iocpData->_buff.len - bytes;
	if (nRemainLen > 0)
	{//����Ϊֹ
		iocpData->_writeBuffer.retrieve(bytes);
		iocpData->_buff.len = iocpData->_writeBuffer.readableBytes();
		iocpData->_buff.buf = iocpData->_writeBuffer.beginRead();
		DWORD flags = 0;
		DWORD byteSended;
		if (WSASend(GetSocketId(), &iocpData->_buff, 1, &byteSended, flags, (LPOVERLAPPED)iocpData, NULL))
		{
			DWORD lastErrorCode = GetLastError();
			if (lastErrorCode != ERROR_IO_PENDING)
			{
				LOG_ERROR << "IOCP WSASend ErrorCode=" << lastErrorCode;
				delete iocpData;
			}
		}
	}
	else
	{
		delete iocpData;

		//�������
	}
}
