#include "../stdafx.h"
#include "InetAddress.h"

InetAddress::InetAddress()
{

}

InetAddress::InetAddress(WORD port, bool loopbackOnly /*= false*/)
{
	Init(port, loopbackOnly);
}

InetAddress::InetAddress(std::string ip, WORD port)
{
	Init(ip, port);
}

InetAddress::InetAddress(SOCKADDR_IN &addr)
{
	memcpy(&_sockAddr, &addr, sizeof SOCKADDR_IN);
}

void InetAddress::Init(WORD port, bool loopbackOnly/* = false*/)
{
	memset(&_sockAddr, 0, sizeof _sockAddr);
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = loopbackOnly ? htonl(INADDR_LOOPBACK) : htonl(INADDR_ANY);
	_sockAddr.sin_port = htons(port);
}

void InetAddress::Init(std::string ip, WORD port)
{
	memset(&_sockAddr, 0, sizeof _sockAddr);
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	_sockAddr.sin_port = htons(port);
}

WORD const InetAddress::GetPort()
{
	return ntohs(_sockAddr.sin_port);
}

std::string InetAddress::GetIp()
{
	return inet_ntoa(_sockAddr.sin_addr);
}

