#ifndef THREAD_EXIT_H
#define  THREAD_EXIT_H

#include <Windows.h>
#include <iostream>
#include <functional>


class CThreadExit
{
public:
	CThreadExit();
	~CThreadExit();

	void SetExitEvent();

	void WaitExit();

private:
	HANDLE _exitEvent;
};
#endif