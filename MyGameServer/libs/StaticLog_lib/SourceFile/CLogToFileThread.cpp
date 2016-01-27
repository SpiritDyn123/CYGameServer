#include "CLogToFileThread.h"
#include "CFile.h"

CLogToFileThread::CLogToFileThread(std::string fileName, int rolFileSize, int flushInterval/* = 3*/) :
_fileName(fileName),
_rolFileSize(rolFileSize),
_flushInterval(flushInterval),
_running(false),
_thread(std::bind(&CLogToFileThread::threadFunc, this, std::placeholders::_1))
{
}

CLogToFileThread::~CLogToFileThread()
{
	if (_running)
		Stop();
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
	_thread.Start();
}

void CLogToFileThread::Stop()
{
	if (!_running)
		return;

	_running = false;
	_thread.Stop();

	CloseHandle(_mutex);
	CloseHandle(_readyEvent);
}

void CLogToFileThread::threadFunc(void *data)
{
	CLogFile output(_fileName, _rolFileSize);
	VEC_LOGBUFFER vecWriteBuff;

	while (_running)
	{
		WaitForSingleObject(_mutex, INFINITE);
		if (_vecBuff.empty())
		{
			ReleaseMutex(_mutex);

			WaitForSingleObject(_readyEvent, _flushInterval * 1000);

			WaitForSingleObject(_mutex, INFINITE);
			if (_currentBuffer && _currentBuffer->length() > 0)
			{
				_vecBuff.push_back(_currentBuffer);
				_currentBuffer = NULL;
				vecWriteBuff.swap(_vecBuff);
			}
			ReleaseMutex(_mutex);
		}
		else
		{
			if (_currentBuffer && _currentBuffer->length() > 0)
			{
				_vecBuff.push_back(_currentBuffer);
				_currentBuffer = NULL;
			}

			vecWriteBuff.swap(_vecBuff);

			ReleaseMutex(_mutex);
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

	return ;
}
