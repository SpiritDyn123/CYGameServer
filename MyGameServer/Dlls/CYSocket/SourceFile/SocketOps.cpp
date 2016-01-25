#include "../stdafx.h"
#include "SocketOps.h"

SOCKET CreateTcpSocket(bool blokcing/* = false*/)
{
	WSADATA wsaData;
	
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
	{
		std::cout << "WSAStartup failed. Error:" << GetLastError() << std::endl;
		return INVALID_SOCKET;
	}

	SOCKET _socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_socket == INVALID_SOCKET)
	{
		std::cout << " WSASocket( listenSocket ) failed. Error:" << GetLastError() << std::endl;
		return INVALID_SOCKET;
	}

	ULONG _blockData = blokcing ? 0 : 1;
	if (ioctlsocket(_socket, FIONBIO, &_blockData) == SOCKET_ERROR)
	{
		std::cout << " ioctlsocket() failed. Error:" << GetLastError() << std::endl;
		return INVALID_SOCKET;
	}

	int nOptBuffSet = MSG_BUFF_LEN;
	setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char *)&nOptBuffSet, sizeof(int));
	setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char *)&nOptBuffSet, sizeof(int));

	return _socket;
}
