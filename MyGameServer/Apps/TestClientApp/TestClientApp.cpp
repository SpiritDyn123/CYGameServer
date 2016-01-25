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

using namespace std;



#include <queue>

#pragma comment(lib, "..\\..\\..\\Bin\\StaticLog_lib.lib")

using namespace std;

#define MAX_CONNECTS_NUM 1024

class MainLogicThread;

MainLogicThread *g_mainLogicTrhead = NULL;
IClientSocket *g_clientSocket = NULL;
class CClientConnectSink : public IClientSocketSink
{
public:
	CClientConnectSink()
	{

	}

	virtual void OnRecv(const char* Buf, int nLen)
	{

	}

	virtual void OnCloseConnect()
	{
		LOG_INFO << "OnCloseConnect 重连中";
		g_clientSocket->Close();
		g_clientSocket->Start("127.0.0.1", 5555);
	}

	virtual void OnConnect(bool bError)
	{
		LOG_INFO << "OnConnect" << bError;
		Sleep(3000);

		g_clientSocket->Send("hello world", 12);
	}

private:
};


class MainLogicThread : public IEventSink
{
	friend class CConnectAcceptSink;
	friend class CConnectSink;

public:
	MainLogicThread() :
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

		if (!sockSys->Create("./Logs/ClientSySocket"))
		{
			LOG_ERROR << "sockSys Create()";
			return;
		}

		ILogicThread *logThread = sockSys->GetLogicThread();
		if (!logThread)
		{
			return;
		}

		logThread->AddEvent(_startEvent, this);

		opMode = omStartClient;
		SetEvent(_startEvent);
	}

	void Release()
	{
		ISocketSysterm *sockSys = _sockSysHelper.GetSocketSys();
		if (sockSys == NULL)
		{
			return;
		}

		opMode = omCloseClient;
		SetEvent(_startEvent);

		for (int i = 0; i < 3; i++)
		{
			Sleep(1000);
			LOG_INFO << "关闭客户端 " << i + 1 << "s...";
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
		if (opMode == omStartClient)
		{
			StartClient();
		}
		else if (opMode == omCloseClient)
		{
			CloseClient();
		}
	}


private:
	//main logic thread func enter 
	void StartClient()
	{
		LOG_INFO << "main logic thread id:" << GetCurrentThreadId();

		ISocketSysterm *sockSys = _sockSysHelper.GetSocketSys();
		if (sockSys == NULL)
		{
			LOG_ERROR << "StartClient GetSocketSys";
			return;
		}

		g_clientSocket = sockSys->CreateClientSocket(&_clientSink);
		if (!g_clientSocket)
		{
			LOG_ERROR << "StartClient CreateClientSocket";
			return;
		}

		if (!g_clientSocket->Start("127.0.0.1", 5555))
		{
			LOG_ERROR << "StartClient g_clientSocket start(127.0.0.1 5555)";
			return;
		}
	}


	void CloseClient()
	{
		if (g_clientSocket)
		{
			g_clientSocket->Close();
			g_clientSocket = NULL;
		}
	}

private:
	HANDLE _startEvent;
	CSocketSystermHepler _sockSysHelper;
	CClientConnectSink _clientSink;

	enum OperateMode
	{
		omNone,
		omStartClient,
		omCloseClient,
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

//  	CTestHelper testheper;
//  	testheper.Create();
//  	ITestSys *pTestSys = testheper.GetTestsys();
//  	if (pTestSys)
//  	{
//  		pTestSys->CreateThread();
//  		Sleep(10000);
//  		pTestSys->StopThread();
//  	}

	system("pause");

	mait.Release();
	return 0;
}
