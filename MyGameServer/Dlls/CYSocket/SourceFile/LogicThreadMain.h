#ifndef LOGIC_THREAD_MAIN_H
#define  LOGIC_THREAD_MAIN_H

#include <Windows.h>
#include <iostream>

#include "ISocketSysterm.h"
#include "CLog.h"
#include "CLogger.h"
#include <map>
#include <vector>
#include <queue>
#include <functional>
#include "CThread.h"
#include "CConnectionChannel.h"

//typedef void(*runInLoopFun)();
typedef std::function<void()> runInLoopFun;

class CSocketSystem;

class LogicThreadMain : public ILogicThread
{
public:
	LogicThreadMain(CSocketSystem *sockSys);

	~LogicThreadMain();

	bool Create();

	void Release();

	bool IsRun() const { return _running; }

	CSocketSystem* GetSocketSys() { return _socketSys; }

	bool IsRunInLoop() { return _mainLogicThread.IsRun() && GetCurrentThreadId() == _mainLogicThread.GetThreadId(); }

	void RunInLoop(runInLoopFun func);

public:
	virtual bool AddEvent(HANDLE hEvent, IEventSink *eventSink);

	virtual void RemoveEvent(HANDLE hEvent);

	struct EventItem
	{
		HANDLE _ev = NULL;
		IEventSink *_sink = NULL;
	};

private:
	void threadLoopFun(void *data);

	void processEvent();
	void processPendingFuncs();
	void processServerMsgs(std::queue<TcpMsgPtr>& queMsgs, std::map<DWORD, IClientSink *>&);
	void processClientMsgss(std::queue<TcpMsgPtr>& queMsgs);

private:
	bool _running;
	CThread _mainLogicThread;
	HANDLE _mutex;

	std::vector<EventItem> _vecEvents;

	std::vector<runInLoopFun> _appendLoopFuncs;
	CSocketSystem *_socketSys;
};

#endif