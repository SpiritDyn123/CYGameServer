#include "../stdafx.h"
#include "CThread.h"

namespace detail
{
	struct ThreadData
	{
		CThread *_threadObj;
		CThread::ThreadFun _func;
		void *data;
	};

	DWORD WINAPI threadFunc(LPVOID lp)
	{
		ThreadData *data = (ThreadData *)lp;
		if (data == NULL)
			return 0;

		data->_func(data->data);

		data->_threadObj->SetExitEvent();

		delete data;

		return 0;
	}
}

CThread::CThread(ThreadFun func) :
_running(false),
_threadHd(NULL),
_threadId(0),
_func(func)
{

}

CThread::~CThread()
{

}

bool CThread::Start(void *data/* = NULL*/)
{
	if (_running)
		return true;

	_running = true;
	detail::ThreadData *tdata = new detail::ThreadData;
	tdata->_threadObj = this;
	tdata->data = data;
	tdata->_func = _func;

	_threadHd = CreateThread(NULL, 0, detail::threadFunc, tdata, 0, &_threadId);
	if (!_threadHd)
	{
		_running = false;
		delete tdata;
		return false;
	}

	return true;
}

void CThread::Stop()
{
	if (!_running)
		return;

	_running = false;
	WaitExit();//一直等待线程中发出退出的event,以弥补dll中不能调用结束线程的WaitSignelObject等待死锁的问题
	CloseHandle(_threadHd);
	_threadHd = NULL;
	_threadId = 0;
}
