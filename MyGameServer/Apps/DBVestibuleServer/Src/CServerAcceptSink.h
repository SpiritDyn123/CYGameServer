#ifndef CSERVER_ACCEPT_SINK_H
#define CSERVER_ACCEPT_SINK_H
#include <Windows.h>
#include <ISocketSysterm.h>
#include "CConnectionSinkPool.h"

class CServerAcceptSink : public IServerSocketSink
{
public:
	virtual void OnAccept(DWORD conId, IClientSink **pSink);

	CConnectionSinkPool *GetConnectionPool() { return &_connsPool; }

private:
	CConnectionSinkPool _connsPool;
};

#endif