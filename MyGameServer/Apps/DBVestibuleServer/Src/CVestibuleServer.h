#ifndef CVESTIBULUE_SERVER_H
#define  CVESTIBULUE_SERVER_H

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "ISocketSysterm.h"
#include "ILogDefines.h"
#include "mysql_connection.h"
#include "mysql_driver.h" 
#include "cppconn/driver.h"  
#include "cppconn/statement.h"  
#include "cppconn/prepared_statement.h"  
#include "cppconn/metadata.h"  
#include "cppconn/exception.h"  

#include "CIniFile.h"
#include "CServerAcceptSink.h"

using namespace sql;
class CVestibuleServer
{	
public:
	CVestibuleServer();

	~CVestibuleServer();

	bool StartServer(ISocketSysterm *);

	bool CloseServer(ISocketSysterm *);

private:
	IServerSocket *_pServerSocket;
	mysql::MySQL_Driver *_sqlDriver;
	Connection *_sqlCon;
	Statement *_sqlState;
	CIniFile _serverInis;
	CIniFile _mysqlInis;
	CServerAcceptSink _acceptSink;
};
#endif