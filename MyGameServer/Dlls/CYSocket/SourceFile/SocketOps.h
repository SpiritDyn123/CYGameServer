#ifndef SOCKET_OPS_H
#define SOCKET_OPS_H

#include <Windows.h>
#include "CLogger.h"

#define MSG_BUFF_LEN 1024 * 500 //单条消息长度不能大于500k
#define SOCKET_BUFF_LEN 65535	//socket缓存是64K-1

#define DEFAUT_MAX_CONN 1024

#define TCP_SERVER_LISTEN_COUNT 5

SOCKET CreateTcpSocket(bool blokcing = false);

#endif