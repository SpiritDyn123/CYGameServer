#ifndef CCONNECTION_SINK_POOL_H
#define CCONNECTION_SINK_POOL_H
#include <Windows.h>
#include <vector>
#include <ISocketSysterm.h>
#include "ILogDefines.h"

class CConnectionSink : public IClientSink
{
public:
	virtual void OnRecv(const char* Buf, int nLen);
	virtual void OnCloseConnect();

	void SetConnId(DWORD connId);

private:
	DWORD _connId;
};

class CConnectionSinkPool
{
public:
	void Init(int nMaxConn);
	void UnInit();

	CConnectionSink *GetIdleSink();

private:
	std::vector<CConnectionSink *> _idleConns;
	std::vector<CConnectionSink *> _usedConns;
};

#endif