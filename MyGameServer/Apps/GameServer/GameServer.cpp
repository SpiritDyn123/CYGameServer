// GameServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN
#include "ILog.h"
#include <string.h>

#include "CLog.h"
#include "CLogger.h"

#include "ISocketSysterm.h"
#include <iostream>
#include <functional>
#include "./Src/TimerWheel.h"

using namespace std;



#include <queue>

#pragma comment(lib, "..\\..\\..\\Bin\\StaticLog_lib.lib")

using namespace std;

#define MAX_CONNECTS_NUM 1024

class MainLogicThread;

MainLogicThread *g_mainLogicTrhead = NULL;
IServerSocket *g_serverSocket = NULL;

class CConnectSink : public IClientSink
{
public:

	virtual void OnRecv(const char* Buf, int nLen)
	{
		Buf; nLen;
		static char data[65535 * 500] = {0};
		memset(data, 0, sizeof data);
		memcpy(data, Buf, nLen);
		LOG_INFO << "coonid=" << socketId << ", recv buf:" << data;

		char ip[128] = { 0 };
		WORD port;
		g_serverSocket->GetClientInfo(socketId, ip, port);
		//if (g_serverSocket)
			//g_serverSocket->Send(socketId, (char *)Buf, nLen);
		g_serverSocket->CloseSocket(socketId);
	}

	virtual void OnCloseConnect()
	{
		SetIdle(true);
	}

public:
	CConnectSink() :
		bIdle(true)
	{

	}

	void SetIdle(bool idle)
	{
		bIdle = idle;
	}

	bool IsIdle()
	{
		return bIdle;
	}

	void SetSocketId(DWORD id)
	{
		socketId = id;
	}

private:
	DWORD socketId;
	bool bIdle;
};

class CConnectSinkPool
{
	friend class CConnectSink;

public:
	CConnectSinkPool()
	{
		for (int i = 0; i < MAX_CONNECTS_NUM; i++)
		{
			_arrConnects[i] = new CConnectSink();
		}
	}

	~CConnectSinkPool()
	{
		for (int i = 0; i < MAX_CONNECTS_NUM; i++)
		{
			delete _arrConnects[i];
			_arrConnects[i] = NULL;
		}
	}


	IClientSink * GetIdleClientSink()
	{
		for (int i = 0; i < MAX_CONNECTS_NUM; i++)
		{
			if (_arrConnects[i] && _arrConnects[i]->IsIdle())
			{
				return _arrConnects[i];
			}
		}

		return NULL;
	}

private:
	CConnectSink *_arrConnects[MAX_CONNECTS_NUM];
};

class CConnectAcceptSink : public IServerSocketSink
{
public:
	CConnectAcceptSink(MainLogicThread *maint) :
		_mainThread(maint)
	{

	}
	virtual void OnAccept(DWORD sockId, IClientSink **pSink)
	{
		CConnectSink *sink = dynamic_cast<CConnectSink *>(_connectPool.GetIdleClientSink());
		if (!sink)
		{
			if (g_serverSocket)
			{
				char *data = "服务器已经达到连接最大数";
				g_serverSocket->Send(sockId, data, strlen(data) + 1);
			}
			return;
		}

		sink->SetIdle(false);
		sink->SetSocketId(sockId);
		*pSink = sink;
	}

private:
	CConnectSinkPool _connectPool;
	MainLogicThread *_mainThread;
};


class MainLogicThread : public IEventSink
{
	friend class CConnectAcceptSink;
	friend class CConnectSink;

public:
	MainLogicThread() :
		_acceptSink(this),
		_pServerSocket(NULL),
		opMode(omNone)
	{

	}

	void Create()
	{
		_startEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (_startEvent == NULL)
		{

			return;
		}

		_sockSysHelper.Create();

		ISocketSysterm *sockSys = _sockSysHelper.GetSocketSys();
		if (sockSys == NULL)
		{
			LOG_ERROR << "_sockSysHelper.GetSocketSys()";
			return;
		}

		if (!sockSys->Create("./Logs/ServerSySocket"))
		{
			LOG_ERROR << "_sockSys.Create()";
			return;
		}

		ILogicThread *logThread = sockSys->GetLogicThread();
		if (!logThread)
		{
			return;
		}

		logThread->AddEvent(_startEvent, this);

		opMode = omStartServer;
		SetEvent(_startEvent);
	}

	void Release()
	{
		ISocketSysterm *sockSys = _sockSysHelper.GetSocketSys();
		if (sockSys == NULL)
		{
			return;
		}


		opMode = omCloseServer;
		SetEvent(_startEvent);

		for (int i = 0; i < 3; i++)
		{
			Sleep(1000);
			LOG_INFO << "关闭服务器中 " << i+1 << "s...";
		}

		ILogicThread *logThread = sockSys->GetLogicThread();
		if (logThread)
		{
			logThread->RemoveEvent(_startEvent);
		}

		sockSys->Release();
	
	}

	virtual void OnEvent()
	{
		if (opMode == omStartServer)
		{
			StartServer();
		}
		else if (opMode == omCloseServer)
		{
			CloseServer();
		}
	}

	
private:
	//main logic thread func enter 
	void StartServer()
	{
		LOG_INFO << "main logic thread id:" << GetCurrentThreadId();

		ISocketSysterm *sockSys = _sockSysHelper.GetSocketSys();
		if (sockSys == NULL)
		{
			LOG_ERROR << "startServer GetSocketSys";
			return;
		}

		_pServerSocket = sockSys->CreateServerSocket(&_acceptSink);
		if (!_pServerSocket)
		{
			LOG_ERROR << "startServer CreateServerSocket";
			return;
		}

		if (!_pServerSocket->Start(5555, 1024))
		{
			LOG_ERROR << "startServer _pServerSocket start(5555)";
			return;
		}

		g_serverSocket = _pServerSocket;
	}


	void CloseServer()
	{
		if (_pServerSocket)
		{
			_pServerSocket->Close();
			_pServerSocket = NULL;
		}

		g_serverSocket = NULL;
	}

private:
	HANDLE _startEvent;
	CSocketSystermHepler _sockSysHelper;
	IServerSocket *_pServerSocket;
	CConnectAcceptSink _acceptSink;

	enum OperateMode
	{
		omNone,
		omStartServer,
		omCloseServer,
	};
	OperateMode opMode;

};

#include "ITest.h"

bool runing;
HANDLE g_thread;
HANDLE g_event;
DWORD WINAPI ThreadFunc(LPVOID lp)
{
	while (runing)
	{
		DWORD ret = WaitForSingleObject(g_event, 0);
		if (ret == WAIT_OBJECT_0)
		{
			Sleep(3000);
			LOG_INFO << "thread wait event success";
			//SetEvent(g_event);
		}
	}
	//Sleep(3000);
	return 0;
}					

int _tmain(int argc, _TCHAR* argv[])
{
	LOG_INFO << "main thread id:" << GetCurrentThreadId();
	MainLogicThread mait;
	mait.Create();

// 	runing = true;
// 	g_event = CreateEvent(NULL, FALSE, FALSE, NULL);
// 	g_thread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);
// 	SetEvent(g_event);
// 	Sleep(1000);
// 	int ret = WaitForSingleObject(g_event,INFINITE);


 	system("pause");
// 
 	mait.Release();
	return 0;
}

