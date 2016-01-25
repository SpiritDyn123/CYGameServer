#include "stdafx.h"
#include "CConnectionSinkPool.h"
#include "CLogger.h"


void CConnectionSink::OnRecv(const char* Buf, int nLen)
{

}

void CConnectionSink::OnCloseConnect()
{

}

void CConnectionSink::SetConnId(DWORD connId)
{
	_connId = connId;
}


void CConnectionSinkPool::Init(int nMaxConn)
{
	if (nMaxConn <= 0)
		return;

	_idleConns.clear();
	_usedConns.clear();
	for (int i = 0; i < nMaxConn; i++)
	{
		_idleConns.push_back(new CConnectionSink);
	}
}

void CConnectionSinkPool::UnInit()
{
	for (std::vector<CConnectionSink *>::iterator iter = _idleConns.begin();
		iter != _idleConns.end();
		++iter)
	{
		if (*iter)
		{
			delete *iter;
		}
	}
	_idleConns.clear();

	for (std::vector<CConnectionSink *>::iterator iter = _usedConns.begin();
		iter != _usedConns.end();
		++iter)
	{
		if (*iter)
		{
			delete *iter;
		}
	}
	_usedConns.clear();

}

CConnectionSink *CConnectionSinkPool::GetIdleSink()
{
	if (_idleConns.size() > 0)
	{
		CConnectionSink *pSink = *(_idleConns.erase(_idleConns.begin()));
		if (pSink)
		{
			_usedConns.push_back(pSink);
		}
		return pSink;
	}

	return NULL;
}