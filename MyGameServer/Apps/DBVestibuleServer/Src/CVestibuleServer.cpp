#include "stdafx.h"
#include "CVestibuleServer.h"
#include "CServerAcceptSink.h"
CVestibuleServer::CVestibuleServer() :
_serverInis("./ServerInis/Servers.ini"),
_mysqlInis("./ServerInis/DBSetting.ini")
{

}

CVestibuleServer::~CVestibuleServer()
{

}

bool CVestibuleServer::StartServer(ISocketSysterm * sockSys)
{
	if (!sockSys)
	{
		LOG_ERROR << "启动服务器失败 sockSys==NULL";
		return false;
	}

	//启动mysql
	_sqlDriver = mysql::get_mysql_driver_instance();
	if (!_sqlDriver)
	{
		LOG_ERROR << "获取sqldriver失败";
		return false;
	}

	_sqlCon = _sqlDriver->connect(_mysqlInis.GetStrKeyValue("mysql", "ipport")
		, _mysqlInis.GetStrKeyValue("mysql", "username")
		, _mysqlInis.GetStrKeyValue("mysql", "password"));
	if (!_sqlCon)
	{
		LOG_ERROR << "连接sql失败";
		return false;
	}

	_sqlState = _sqlCon->createStatement();
	if (!_sqlCon)
	{
		LOG_ERROR << "创建sqlstate失败";
		return false;
	}

	//启动socketserver
	_pServerSocket = sockSys->CreateServerSocket(&_acceptSink);
	if (!_pServerSocket)
	{
		LOG_ERROR << "创建服务器socket失败";
		return false;
	}

	int port = _serverInis.GetIntKeyValue("DbVestibule", "port");
	int maxConns = _serverInis.GetIntKeyValue("DbVestibule", "maxConns");

	_acceptSink.GetConnectionPool()->Init(maxConns);

	if (!_pServerSocket->Start((WORD)port, maxConns))
	{
		LOG_ERROR << "启动服务器失败，port=" << _serverInis.GetIntKeyValue("DbVestibule", "port");
		return false;
	}

	LOG_INFO << "启动服务器成功";
	return true;
}

bool CVestibuleServer::CloseServer(ISocketSysterm * sockSys)
{
	if (!sockSys)
	{
		LOG_ERROR << "关闭服务器失败 sockSys==NULL";
		return false;
	}

	sockSys->Release();

	_acceptSink.GetConnectionPool()->UnInit();

	delete _sqlState;
	delete _sqlCon;
	delete _sqlDriver;

	return true;
}