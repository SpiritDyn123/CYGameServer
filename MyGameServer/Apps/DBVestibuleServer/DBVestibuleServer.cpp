// DBVestibuleServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "./Src/CInitServerModule.h"

#include "ILogDefines.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CInitServerModule server;
	server.Create();

 	system("pause");
 	server.Release();
	return 0;
}

