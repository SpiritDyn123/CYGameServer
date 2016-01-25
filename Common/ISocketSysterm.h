#ifndef ISOCKET_SYSTERM_H
#define ISOCKET_SYSTERM_H

#include <Windows.h>
#include <iostream>
#include "IThreadQueue.h"

class IServerSocket
{
public:
	virtual bool Start(WORD port, int MaxConnNum) = NULL;

	virtual void Close() = NULL;

	virtual void Send(DWORD conId, char *data, int nLen) = NULL;

	virtual bool GetClientInfo(DWORD conId, char *szIp, WORD &port) = NULL;

	virtual void CloseSocket(DWORD conId) = NULL;
};

class IClientSocket
{
public:
	virtual bool Start(std::string ip, WORD port) = NULL;

	virtual void Close() = NULL;

	virtual void Send(char *data, int nLen) = NULL;
};

class IClientSink
{
public:
	virtual void OnRecv(const char* Buf, int nLen) = NULL;
	virtual void OnCloseConnect() = NULL;
};

class IServerSocketSink
{
public:
	virtual void OnAccept(DWORD conId, IClientSink **pSink) = NULL;
};

class IClientSocketSink
{
public:
	virtual void OnRecv(const char* Buf, int nLen) = NULL;
	virtual void OnCloseConnect() = NULL;
	virtual void OnConnect(bool bError) = NULL;
};


class IEventSink
{
public:
	virtual void OnEvent() = NULL;
};

class ILogicThread
{
public:
	virtual bool AddEvent(HANDLE hEvent, IEventSink *eventSink) = NULL;
	virtual void RemoveEvent(HANDLE hEvent) = NULL;
};

class ISocketSysterm
{
public:
	virtual ILogicThread *GetLogicThread() = NULL;
	virtual IServerSocket * CreateServerSocket(IServerSocketSink *pServerSocketSink) = NULL;
	virtual IClientSocket * CreateClientSocket(IClientSocketSink *pClientSocketSink) = NULL;

	virtual bool Create(std::string fileName) = NULL;

	virtual void Release() = NULL;
	//test
	//virtual IThreadPool * CreateThreadPool() = NULL;
};

class CSocketSystermHepler
{
public:
	typedef bool(*DllPreedFunc)(ISocketSysterm **pSocketSys);
	CSocketSystermHepler() :
		_hDll(NULL),
		_socketSys(NULL)
	{

	}

	~CSocketSystermHepler()
	{
		Release();
	}

	void Create()
	{
		Release();
		try
		{
			_hDll = LoadLibrary("CYSocket.dll");
			if (_hDll == NULL)
			{
				throw "Can't load CYSocket.dll";
			}

			DllPreedFunc proc = (DllPreedFunc)GetProcAddress(_hDll, "CreateSocketSystem");
			if (proc == NULL)
				throw "Can't GetProcAddress('CreateSocketSystem')";

			proc(&_socketSys);
		}
		catch (LPCSTR errMgs)
		{
			throw errMgs;
		}
	}

	void Release()
	{
		if (_hDll != NULL)
		{
			::FreeLibrary(_hDll);
			_hDll = NULL;
		}
	}

	ISocketSysterm * operator ->()
	{
		return _socketSys;
	}

	ISocketSysterm * GetSocketSys()
	{
		return _socketSys;
	}

private:
	HINSTANCE _hDll;
	ISocketSysterm *_socketSys;
};
#endif