#ifndef TIMER_WHEEL_H
#define  TIMER_WHEEL_H

#include <Windows.h>
#include <iostream>
#include <set>
#include <vector>
#include <ISocketSysterm.h>
#include <iterator>
class ITimer
{
public:
	virtual void OnTimer(DWORD nowTick) = NULL;
};

struct TimerEntity
{
	ITimer *_timer;
	DWORD _when;//何时启动,
	DWORD _interval;//启动间隔，若为0则为一次性的
};
class TimerWheel : public IEventSink
{
public:
	TimerWheel();

	~TimerWheel();

	virtual void OnEvent();

	bool AddTimer(ITimer *timer, DWORD when = 0, DWORD interval = 0);

	void DelTimer(ITimer *);


private:
	typedef std::pair<DWORD, TimerEntity*> Entity;
	typedef std::set<Entity> TimerList;
	TimerList _allTimers;
};
#endif