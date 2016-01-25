#ifndef ITHREAD_QUEUE_H
#define ITHREAD_QUEUE_H

#include <Windows.h>
#include <iostream>

class IThreadWork
{
public:
	virtual void DoTask(void *data = NULL) = NULL;
};

class IThreadQueue
{
public:
	virtual bool Create(int nThreadNum) = NULL;
	virtual void Release(int nWaitMSec = 0) = NULL;//д╛хо=0
	virtual bool InsertThreadWorkObj(IThreadWork *pWorkItem) = NULL;
};


#endif