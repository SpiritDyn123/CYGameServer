#include "stdafx.h"
#include "CThread.h"

namespace detail
{
	struct ThreadData
	{
		CThread *_objThread;
		CThread::ThreadFun _func;
		void *data;
	};

	DWORD WINAPI threadFunc(LPVOID lp)
	{
		ThreadData *data = (ThreadData *)lp;
		if (data == NULL)
			return 0;

		data->_func(data->data);

		Sleep(5000);
		data->_objThread->SetExitEvent();

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

void CThread::start(void *data/* = NULL*/)
{
	if (_running)
		return;

	_running = true;
	detail::ThreadData *tdata = new detail::ThreadData;
	tdata->_objThread = this;
	tdata->data = data;
	tdata->_func = _func;
	_threadHd = CreateThread(NULL, 0, detail::threadFunc, tdata, 0, &_threadId);
	if (!_threadHd)
	{
		_running = false;
		delete tdata;
	}
}

void CThread::stop()
{
	if (!_running)
		return;

	_running = false;
	WaitExit();
	CloseHandle(_threadHd);
	_threadHd = NULL;
	_threadId = 0;
}
