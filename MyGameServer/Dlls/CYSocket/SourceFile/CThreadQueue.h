#ifndef CTHREAD_QUEUE_H
#define CTHREAD_QUEUE_H

#include <list>
#include <queue>
#include <vector>
#include <Windows.h>
#include "CThread.h"

#include "IThreadQueue.h"

#define THREAD_POOL_MAX_NUM 20
#define THREAD_POOL_MAX_IDLE_NUM 10

class CThreadQueue : public IThreadQueue
{
public:
	CThreadQueue();
	~CThreadQueue(){}

	virtual bool Create(int nThreadNum);
	virtual void Release(int nWaitMSec = 0);//默认=0，无限等待
	virtual bool InsertThreadWorkObj(IThreadWork *pWorkItem);
	
private://线程函数使用
	IThreadWork * GetNextWorkObj();
	bool UpdateWork();

	void threadWorkFun(void *data);
	void threadUpdateWorkFun(void *data);

private:
	void ExistAllThread(int nWaitMSec = 0);
	int GetWaitWorkObjCount();
public:
	enum
	{
		CON_WORK_SEMOPH_INDEX,
		CON_ABORT_EVENT_INDEX,
		CON_COUNT
	};

	HANDLE _threadConArr[CON_COUNT];//创建处罚任务信号量和终止线程event
private:

	HANDLE _mutex;
	std::queue<IThreadWork *> _threadWorkObjQueue;//工作任务queue

	int _nThreadNum; //线程数

	CThread *_updateThread;//调度线程
	std::vector<CThread *> _idleThreads;//空闲线程队列

	bool _running;
};

#endif