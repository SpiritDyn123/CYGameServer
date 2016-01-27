#ifndef THREAD_H
#define  THREAD_H

#include <Windows.h>
#include <iostream>
#include <functional>
#include "CThreadExit.h"

class CThread : public CThreadExit
{
public:
	typedef std::function<void(void *)> ThreadFun;

	CThread(ThreadFun func);
	~CThread();

	bool Start(void *data = NULL);

	void Stop();

	bool IsRun() const { return _running; }

	DWORD GetThreadId() const { return _threadId; }

private:
	ThreadFun _func;
	DWORD _threadId;
	HANDLE _threadHd;
	bool _running;
};
#endif