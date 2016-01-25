#include "CLogToFileThread.h"
#include "CFile.h"

CLogToFileThread::CLogToFileThread(std::string fileName, int rolFileSize, int flushInterval/* = 3*/) :
_fileName(fileName),
_rolFileSize(rolFileSize),
_flushInterval(flushInterval),
_running(false)
{
	_exitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(_exitEvent);
}

CLogToFileThread::~CLogToFileThread()
{
	if (_running)
		Stop();

	CloseHandle(_exitEvent);
}

void CLogToFileThread::append(const char *data, int nLen)
{
	if (!_running)
		return;

	if (data == NULL || nLen <= 0)
		return;

	WaitForSingleObject(_mutex, INFINITE);
	if (_currentBuffer == NULL)
		_currentBuffer = new LogBuffer;
	if (_currentBuffer->avail() >= nLen)
	{
		_currentBuffer->append(data, nLen);
	}
	else
	{
		_vecBuff.push_back(_currentBuffer);
		_currentBuffer = new LogBuffer;
		_currentBuffer->append(data, nLen);
		SetEvent(_readyEvent);
	}
	ReleaseMutex(_mutex);
}

void CLogToFileThread::Start()
{
	if (_running)
		return;

	_mutex = CreateMutex(NULL, FALSE, NULL);
	assert(_mutex != NULL);

	_readyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(_readyEvent != NULL);

	_vecBuff.clear();
	_currentBuffer = NULL;

	_running = true;

	_threadHd = CreateThread(NULL, 0, threadFunc, this, 0, &_threadId);
	assert(_threadId != NULL);
}

void CLogToFileThread::Stop()
{
	if (!_running)
		return;

	_running = false;
	//WaitForSingleObject(_threadHd, INFINITE);
	WaitForSingleObject(_exitEvent, INFINITE);
	CloseHandle(_mutex);
	CloseHandle(_readyEvent);
	CloseHandle(_threadHd);
}

DWORD WINAPI CLogToFileThread::threadFunc(LPVOID lparm)
{
	CLogToFileThread *logMain = (CLogToFileThread *)lparm;
	if (logMain == NULL)
		return 0;

	CLogFile output(logMain->_fileName, logMain->_rolFileSize);
	VEC_LOGBUFFER vecWriteBuff;

	while (logMain->_running)
	{
		WaitForSingleObject(logMain->_mutex, INFINITE);
		if (logMain->_vecBuff.empty())
		{
			ReleaseMutex(logMain->_mutex);

			WaitForSingleObject(logMain->_readyEvent, logMain->_flushInterval * 1000);

			WaitForSingleObject(logMain->_mutex, INFINITE);
			if (logMain->_currentBuffer && logMain->_currentBuffer->length() > 0)
			{
				logMain->_vecBuff.push_back(logMain->_currentBuffer);
				logMain->_currentBuffer = NULL;
				vecWriteBuff.swap(logMain->_vecBuff);
			}
			ReleaseMutex(logMain->_mutex);
		}
		else
		{
			if (logMain->_currentBuffer && logMain->_currentBuffer->length() > 0)
			{
				logMain->_vecBuff.push_back(logMain->_currentBuffer);
				logMain->_currentBuffer = NULL;
			}

			vecWriteBuff.swap(logMain->_vecBuff);

			ReleaseMutex(logMain->_mutex);
		}

		int nWriteBuffNums = (int)vecWriteBuff.size();
		if (nWriteBuffNums > 0)
		{
			LogBuffer *buff = NULL;
			for (int i = 0; i < nWriteBuffNums; i++)
			{
				buff = vecWriteBuff[i];
				if (buff)
				{
					output.append(buff->data(), buff->length());
					delete buff;
					buff = NULL;
				}
			}

			vecWriteBuff.clear();
			output.flush();
		}
	
	}

	output.flush();

	SetEvent(logMain->_exitEvent);

	return 0;
}
