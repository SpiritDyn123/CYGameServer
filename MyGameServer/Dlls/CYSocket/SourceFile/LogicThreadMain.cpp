#include "../stdafx.h"
#include "ISocketSysterm.h"
#include "LogicThreadMain.h"
#include "CSocketSystem.h"
#include <queue>

LogicThreadMain::LogicThreadMain(CSocketSystem *sockSys) :
_socketSys(sockSys),
_running(false),
_mainLogicThread(std::bind(&LogicThreadMain::threadLoopFun, this, std::placeholders::_1)),
_mutex(NULL)
{
}

LogicThreadMain::~LogicThreadMain()
{
	_socketSys = NULL;
}

bool LogicThreadMain::Create()
{
	_running = true;

	_mutex = CreateMutex(NULL, FALSE, NULL);
	if (!_mutex)
	{
		_running = false;
		return false;
	}

	_vecEvents.clear();
	if (!_mainLogicThread.Start())
	{
		_running = false;
		ReleaseMutex(_mutex);
		_mutex = NULL;
		return false;
	}

	return true;
}

void LogicThreadMain::Release()
{
	if (!_running)
		return;

	WaitForSingleObject(_mutex, INFINITE);
	_vecEvents.clear();
	ReleaseMutex(_mutex);

	_running = false;

	_mainLogicThread.Stop();

	CloseHandle(_mutex);
	_mutex = NULL;
}

void LogicThreadMain::RunInLoop(runInLoopFun func)
{
	if (!_running)
		return;

	if (IsRunInLoop())
	{
		func();
	}
	else
	{
		WaitForSingleObject(_mutex, INFINITE);
		_appendLoopFuncs.push_back(func);
		ReleaseMutex(_mutex);
	}
}

bool LogicThreadMain::AddEvent(HANDLE hEvent, IEventSink *eventSink)
{
	if (IsRunInLoop())
	{
		for (std::vector<EventItem>::iterator iter = _vecEvents.begin();
			iter != _vecEvents.end();
			++iter)
		{
			if (iter->_ev == hEvent)
			{
				LOG_ERROR << "AddEvent Exist";
				return false;
			}
		}

		EventItem evItem;
		evItem._ev = hEvent;
		evItem._sink = eventSink;
		_vecEvents.push_back(evItem);
	}
	else
	{
		WaitForSingleObject(_mutex, INFINITE);
		for (std::vector<EventItem>::iterator iter = _vecEvents.begin();
			iter != _vecEvents.end();
			++iter)
		{
			if (iter->_ev == hEvent)
			{
				LOG_ERROR << "AddEvent Exist";
				ReleaseMutex(_mutex);
				return false;
			}
		}

		EventItem evItem;
		evItem._ev = hEvent;
		evItem._sink = eventSink;
		_vecEvents.push_back(evItem);
		ReleaseMutex(_mutex);
	}

	return true;
}

void LogicThreadMain::RemoveEvent(HANDLE hEvent)
{
	if (IsRunInLoop())
	{
		for (std::vector<EventItem>::iterator iter = _vecEvents.begin();
			iter != _vecEvents.end();
			++iter)
		{
			if (iter->_ev == hEvent)
			{
				_vecEvents.erase(iter);
				break;
			}
		}
	}
	else
	{
		WaitForSingleObject(_mutex, INFINITE);
		for (std::vector<EventItem>::iterator iter = _vecEvents.begin();
			iter != _vecEvents.end();
			++iter)
		{
			if (iter->_ev == hEvent)
			{
				_vecEvents.erase(iter);
				break;
			}
		}

		ReleaseMutex(_mutex);
	}
}

void LogicThreadMain::threadLoopFun(void *data)
{
	std::queue<TcpMsgPtr> queMsgs;
	while (!queMsgs.empty())
	{
		queMsgs.pop();
	}

	std::map<DWORD, IClientSink *> mapClients;
	mapClients.clear();

	//整个上层逻辑的线程loop，主要负责其他线程的函数回调_appendLoopFuncs，Event触发（如定时器），serverSocket和clientSocket消息队列
	while (_running)
	{
		//appendingFunc;
		processPendingFuncs();

		//events;
		processEvent();

		//serverSocket queMsgs;
		processServerMsgs(queMsgs, mapClients);

		//解析所有clientsocket消息队列
		processClientMsgss(queMsgs);
	}

	return;
}

void LogicThreadMain::processEvent()
{
	std::vector<EventItem> vecEvents;
	vecEvents.clear();
	WaitForSingleObject(_mutex, INFINITE);
	vecEvents.assign(_vecEvents.begin(), _vecEvents.end());
	ReleaseMutex(_mutex);

	int nEventSize = vecEvents.size();
	if (nEventSize > 0)
	{
		std::vector<HANDLE> vecHandles;
		vecHandles.clear();
		for (int i = 0; i < nEventSize; i++)
		{
			vecHandles.push_back(vecEvents[i]._ev);
		}

		DWORD mode = WaitForMultipleObjects(nEventSize, (HANDLE *)&*vecHandles.begin(), FALSE, 0);
		int nEventIndx = mode - WAIT_OBJECT_0;
		if (nEventIndx >= 0 && nEventIndx < nEventSize)
		{
			IEventSink * sink = vecEvents[nEventIndx]._sink;
			if (sink)
			{
				sink->OnEvent();
			}
		}
	}
}

void LogicThreadMain::processPendingFuncs()
{
	WaitForSingleObject(_mutex, INFINITE);
	for (int i = 0; i < (int)_appendLoopFuncs.size(); i++)
	{
		runInLoopFun func = _appendLoopFuncs[i];
		if (func)
		{
			func();
		}
	}

	_appendLoopFuncs.clear();
	ReleaseMutex(_mutex);
}

void LogicThreadMain::processServerMsgs(std::queue<TcpMsgPtr>& queMsgs, std::map<DWORD, IClientSink *>&mapClients)
{
	CServerSocket *serverSocket;
	while (!queMsgs.empty())
	{
		queMsgs.pop();
	}

	serverSocket = _socketSys->GetServerSocket();
	if (serverSocket && serverSocket->IsRun())
	{
		serverSocket->SwapQueMsg(queMsgs);

		while (!queMsgs.empty())
		{
			TcpMsgPtr msg = queMsgs.front();
			queMsgs.pop();
			if (!msg)
				continue;

			IClientSink *pClientSink = NULL;
			if (msg->_state == TCP_MSG::Accepted)
			{
				IServerSocketSink *serverSink = serverSocket->GetLink();
				if (serverSink)
				{
					serverSink->OnAccept(msg->_connId, &pClientSink);
					if (pClientSink)
					{
						std::map<DWORD, IClientSink *>::iterator iter = mapClients.find(msg->_connId);
						if (iter != mapClients.end())
						{
							LOG_ERROR << "连接已存在";
							mapClients.erase(iter);
							iter->second->OnCloseConnect();
							mapClients.insert(std::pair<DWORD, IClientSink *>(msg->_connId, pClientSink));
						}
						else
							mapClients.insert(std::pair<SOCKET, IClientSink *>(msg->_connId, pClientSink));
					}
				}
			}
			else if (msg->_state == TCP_MSG::Closed)
			{
				std::map<DWORD, IClientSink *>::iterator iter = mapClients.find(msg->_connId);
				if (iter != mapClients.end())
				{
					pClientSink = iter->second;
					if (pClientSink)
					{
						pClientSink->OnCloseConnect();
					}
					mapClients.erase(iter);
				}
			}
			else if (msg->_state == TCP_MSG::ReadBuff)
			{
				std::map<DWORD, IClientSink *>::iterator iter = mapClients.find(msg->_connId);
				if (iter != mapClients.end())
				{
					pClientSink = iter->second;
					if (pClientSink)
					{
						int nLen = msg->_buff.readInt();
						msg->_buff.retrieve(sizeof(int));
						pClientSink->OnRecv(msg->_buff.beginRead(), nLen);
					}
				}
			}
		}
	}
}

void LogicThreadMain::processClientMsgss(std::queue<TcpMsgPtr>& queMsgs)
{
	
	std::vector<CClientSocket *> vecAllClientsockets = _socketSys->GetAllClientSockets();
	for (int i = 0; i < (int)vecAllClientsockets.size(); i++)
	{
		CClientSocket *clientSocket = vecAllClientsockets[i];
		if (clientSocket/* && clientSocket->IsRun()*/)
		{
			clientSocket->SwapQueMsg(queMsgs);

			while (!queMsgs.empty())
			{
				TcpMsgPtr msg = queMsgs.front();
				queMsgs.pop();
				if (!msg)
					continue;

				IClientSocketSink *clientSink = clientSocket->GetLink();
				if (clientSink)
				{
					if (msg->_state == TCP_MSG::Accepted)
					{
						clientSink->OnConnect(true);
					}
					else if (msg->_state == TCP_MSG::Closed)
					{
						clientSink->OnCloseConnect();
					}
					else
					{
						int nLen = msg->_buff.readInt();
						msg->_buff.retrieve(sizeof(int));
						clientSink->OnRecv(msg->_buff.beginRead(), nLen);
					}
				}
			}
		}
	}
}
