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

	void start(void *data = NULL);

	void stop();

private:
	ThreadFun _func;
	DWORD _threadId;
	HANDLE _threadHd;
	bool _running;
};
#endif