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
	virtual void Release(int nWaitMSec = 0);//Ĭ��=0�����޵ȴ�
	virtual bool InsertThreadWorkObj(IThreadWork *pWorkItem);
	
private://�̺߳���ʹ��
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

	HANDLE _threadConArr[CON_COUNT];//�������������ź�������ֹ�߳�event
private:

	HANDLE _mutex;
	std::queue<IThreadWork *> _threadWorkObjQueue;//��������queue

	int _nThreadNum; //�߳���

	CThread *_updateThread;//�����߳�
	std::vector<CThread *> _idleThreads;//�����̶߳���

	bool _running;
};

#endif