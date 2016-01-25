#include "../stdafx.h"
#include "CThreadQueue.h"
#include "CLogger.h"
#include <functional>

CThreadQueue::CThreadQueue() :
_nThreadNum(0),
_updateThread(NULL),
_running(false)
{
	_idleThreads.clear();
}

bool CThreadQueue::Create(int nThreadNum)
{
	if (nThreadNum <= 0)
		return false;

	_nThreadNum = nThreadNum > THREAD_POOL_MAX_NUM ? THREAD_POOL_MAX_NUM : nThreadNum;

	_mutex = CreateMutex(NULL, FALSE, NULL);
	if (_mutex == NULL)
		return false;

	_threadConArr[CON_WORK_SEMOPH_INDEX] = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
	if (_threadConArr[CON_WORK_SEMOPH_INDEX] == NULL)
	{
		CloseHandle(_mutex);
		return false;
	}

	_threadConArr[CON_ABORT_EVENT_INDEX] = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (_threadConArr[CON_ABORT_EVENT_INDEX] == NULL)
	{
		CloseHandle(_mutex);
		CloseHandle(_threadConArr[CON_WORK_SEMOPH_INDEX]);
		return false;
	}
	
	_running = true;
	_updateThread = new CThread(std::bind(&CThreadQueue::threadUpdateWorkFun, this, std::placeholders::_1));
	if (!_updateThread->Start())
	{
		_running = false;

		CloseHandle(_mutex);
		CloseHandle(_threadConArr[CON_WORK_SEMOPH_INDEX]);
		CloseHandle(_threadConArr[CON_ABORT_EVENT_INDEX]);
		delete _updateThread;
		_updateThread = NULL;

		return false;
	}

	_idleThreads.clear();
	for (int i = _nThreadNum; i > 0; i--)
	{
		CThread *workThread = new CThread(std::bind(&CThreadQueue::threadWorkFun, this, std::placeholders::_1));
		if (!workThread->Start())
		{
			Release();
			return false;
		}
		else
		{
			_idleThreads.push_back(workThread);
		}
	}

	return true;
}

void CThreadQueue::Release(int nWaitMSec/* = 0*/)
{
	if (!_running)
		return;

	_running = false;

	ExistAllThread(nWaitMSec);

	CloseHandle(_mutex);//关闭线程锁

	//关闭信号量和event
	for (int i = 0; i < CON_COUNT; i++)
	{
		CloseHandle(_threadConArr[i]);
	}
}

void CThreadQueue::ExistAllThread(int nWaitMSec/* = 0*/)
{
	nWaitMSec = nWaitMSec > 0 ? nWaitMSec : 0;

	//等待线程工作完毕
	while (0 != GetWaitWorkObjCount())
	{
	}

	_updateThread->Stop();
	delete _updateThread;
	_updateThread = NULL;

	if (!SetEvent(_threadConArr[CON_ABORT_EVENT_INDEX]))
	{
		LOG_ERROR << "ExistAllThread SetEvent Abort error:" << GetLastError();
	}

	//ResetEvent(_threadConArr[CON_ABORT_EVENT_INDEX]);

	for (int i = 0; i < (int)_idleThreads.size(); i++)
	{
		CThread *workThread = _idleThreads[i];
		if (workThread)
		{
			workThread->Stop();
			delete workThread;
		}
	}

	_idleThreads.clear();
}

bool CThreadQueue::InsertThreadWorkObj(IThreadWork *pWorkItem)
{
	if (!_running)
		return false;

	WaitForSingleObject(_mutex, INFINITE);
	_threadWorkObjQueue.push(pWorkItem);
	ReleaseMutex(_mutex);

	return true;
}

IThreadWork * CThreadQueue::GetNextWorkObj()
{
	IThreadWork *pWorkObj = NULL;

	WaitForSingleObject(_mutex, INFINITE);

	if (_threadWorkObjQueue.size() > 0)
	{
		pWorkObj = _threadWorkObjQueue.front();
		_threadWorkObjQueue.pop();
	}

	ReleaseMutex(_mutex);

	return pWorkObj;
}

int CThreadQueue::GetWaitWorkObjCount()
{
	int nWaitCount = 0;
	WaitForSingleObject(_mutex, INFINITE);

	nWaitCount = _threadWorkObjQueue.size();
	ReleaseMutex(_mutex);

	return nWaitCount;
}

void CThreadQueue::threadWorkFun(void *data)
{
	DWORD dwWaitResult;
	while (_running)
	{
		dwWaitResult = WaitForMultipleObjects(CON_COUNT, _threadConArr, FALSE, INFINITE);
		switch (dwWaitResult - WAIT_OBJECT_0)
		{
		case CON_WORK_SEMOPH_INDEX:
			{
				 //切换到work线程list

				IThreadWork *pWorkObj = GetNextWorkObj();
				if (pWorkObj)
				{
					pWorkObj->DoTask();
				}

				//切换到idel线程list
			}
			break;
		case CON_ABORT_EVENT_INDEX:
			{
				return;//线程结束
			}
		default:
			break;
		}
	}

	return;
}

void CThreadQueue::threadUpdateWorkFun(void *data)
{
	while (_running)
	{
		int needWorkNum = 0;
		WaitForSingleObject(_mutex, INFINITE);
		needWorkNum = _threadWorkObjQueue.size();
		ReleaseMutex(_mutex);
		if (needWorkNum > 0)
		{
			if (!ReleaseSemaphore(_threadConArr[CON_WORK_SEMOPH_INDEX], 1, NULL))//发送信号通知线程工作
			{
				LOG_ERROR << "ThreadPool UpdateWork Thread ReleaseSemaphore error";
			}
		}
	}

	return;
}