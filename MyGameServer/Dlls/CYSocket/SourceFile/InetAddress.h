#ifndef INET_ADDRESS_H
#define INET_ADDRESS_H

#include <Windows.h>
#include <WinSock2.h>
#include <iostream>

class InetAddress
{
public:
	InetAddress();

	InetAddress(WORD port, bool loopbackOnly = false);

	InetAddress(std::string ip, WORD port);

	InetAddress(SOCKADDR_IN &addr);

	void Init(WORD port, bool loopbackOnly = false);

	void Init(std::string ip, WORD port);

	WORD const GetPort();

	std::string GetIp();

	SOCKADDR_IN * const GetSockAddr() { return &_sockAddr; }

private:
	SOCKADDR_IN _sockAddr;
};
#endif