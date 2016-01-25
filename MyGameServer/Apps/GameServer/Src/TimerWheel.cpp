#include "stdafx.h"
#include "TimerWheel.h"


TimerWheel::TimerWheel()
{
	_allTimers.clear();
}

TimerWheel::~TimerWheel()
{
	for (TimerList::iterator i = _allTimers.begin(); i != _allTimers.end(); ++i)
	{
		assert(i->second);
		delete i->second;
	}

	_allTimers.clear();
}

void TimerWheel::OnEvent()
{
	DWORD nowTick = GetTickCount();

	std::vector<Entity> expiredTimes;
	expiredTimes.clear();

	Entity sentry(nowTick, NULL);
	TimerList::iterator end = _allTimers.lower_bound(sentry);
	assert(end == _allTimers.end() || nowTick < end->first);
	std::copy(_allTimers.begin(), end, back_inserter(expiredTimes));
	_allTimers.erase(_allTimers.begin(), end);

	for (std::vector<Entity>::iterator iter = expiredTimes.begin()
		; iter != expiredTimes.end()
		; ++iter)
	{
		iter->second->_timer->OnTimer(nowTick);
		if (iter->second->_interval != 0)
		{
			iter->second->_when = nowTick + iter->second->_interval;
			_allTimers.insert(Entity(iter->second->_when, iter->second));
		}
		else
		{
			delete iter->second;
		}
	}
}

bool TimerWheel::AddTimer(ITimer *timer, DWORD when/* = 0*/, DWORD interval/* = 0*/)
{
	if (timer == NULL || when < 0 || interval < 0)
		return false;

	for (TimerList::iterator i = _allTimers.begin(); i != _allTimers.end(); ++i)
	{
		TimerEntity *ent = i->second;
		if (ent->_timer == timer)
			return false;
	}

	DWORD nowTick = GetTickCount();
	TimerEntity *ent = new TimerEntity();
	ent->_timer = timer;
	ent->_when = nowTick + when;
	ent->_interval = interval;

	_allTimers.insert(Entity(ent->_when, ent));
	return true;
}

void TimerWheel::DelTimer(ITimer *timer)
{
	for (TimerList::iterator i = _allTimers.begin(); i != _allTimers.end(); ++i)
	{
		TimerEntity *ent = i->second;
		if (ent->_timer == timer)
		{
			_allTimers.erase(i);
			delete ent;
			break;
		}
	}

}
