#include "../stdafx.h"
#include "CThreadExit.h"
#include <assert.h>

CThreadExit::CThreadExit()
{
	_exitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(_exitEvent != NULL);
}

CThreadExit::~CThreadExit()
{
	if (_exitEvent)
	{
		CloseHandle(_exitEvent);
		_exitEvent = NULL;
	}
}

void CThreadExit::SetExitEvent()
{
	SetEvent(_exitEvent);
}

void CThreadExit::WaitExit()
{
	WaitForSingleObject(_exitEvent, INFINITE);
}