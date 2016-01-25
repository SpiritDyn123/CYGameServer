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
		LOG_ERROR << "����������ʧ�� sockSys==NULL";
		return false;
	}

	//����mysql
	_sqlDriver = mysql::get_mysql_driver_instance();
	if (!_sqlDriver)
	{
		LOG_ERROR << "��ȡsqldriverʧ��";
		return false;
	}

	_sqlCon = _sqlDriver->connect(_mysqlInis.GetStrKeyValue("mysql", "ipport")
		, _mysqlInis.GetStrKeyValue("mysql", "username")
		, _mysqlInis.GetStrKeyValue("mysql", "password"));
	if (!_sqlCon)
	{
		LOG_ERROR << "����sqlʧ��";
		return false;
	}

	_sqlState = _sqlCon->createStatement();
	if (!_sqlCon)
	{
		LOG_ERROR << "����sqlstateʧ��";
		return false;
	}

	//����socketserver
	_pServerSocket = sockSys->CreateServerSocket(&_acceptSink);
	if (!_pServerSocket)
	{
		LOG_ERROR << "����������socketʧ��";
		return false;
	}

	int port = _serverInis.GetIntKeyValue("DbVestibule", "port");
	int maxConns = _serverInis.GetIntKeyValue("DbVestibule", "maxConns");

	_acceptSink.GetConnectionPool()->Init(maxConns);

	if (!_pServerSocket->Start((WORD)port, maxConns))
	{
		LOG_ERROR << "����������ʧ�ܣ�port=" << _serverInis.GetIntKeyValue("DbVestibule", "port");
		return false;
	}

	LOG_INFO << "�����������ɹ�";
	return true;
}

bool CVestibuleServer::CloseServer(ISocketSysterm * sockSys)
{
	if (!sockSys)
	{
		LOG_ERROR << "�رշ�����ʧ�� sockSys==NULL";
		return false;
	}

	sockSys->Release();

	_acceptSink.GetConnectionPool()->UnInit();

	delete _sqlState;
	delete _sqlCon;
	delete _sqlDriver;

	return true;
}