#include "stdafx.h"
#include "CIniFile.h"
#include "CLogger.h"

CIniFile::CIniFile(std::string fileName) :
_fileName(fileName)
{
	_fin.open(_fileName.c_str(), std::ios::in | std::ios::_Nocreate);
	assert(_fin.is_open());
	_appValues.clear();

	char lineData[512];
	AppValues::iterator iterApp;
	KeyValues::iterator iterKey;
	while (!_fin.eof())
	{
		memset(lineData, 0, 512);
		_fin.getline(lineData, 512);
		assert(strlen(lineData) < 512);
		std::string strData = lineData;

		//去除所有空格和缩进
		int tabSpaceIndex = 0;
		while ((tabSpaceIndex = strData.find_first_of(' ', tabSpaceIndex)) != std::string::npos)
		{
			strData.erase(strData.begin() + tabSpaceIndex);
		}

		tabSpaceIndex = 0;
		while ((tabSpaceIndex = strData.find_first_of('\t', tabSpaceIndex)) != std::string::npos)
		{
			strData.erase(strData.begin() + tabSpaceIndex);
		}

		int nStrLen = strData.length();
		if (nStrLen >= 2)
		{
 			if (strData[0] == '#')//表示注释
 				continue;

			//判断是否是app
			if (nStrLen > 2 
				&&strData.find_first_of('[') == 0
				&& strData.find_first_of(']') == nStrLen - 1)
			{
				strData.erase(strData.begin());
				int nEndIndex = nStrLen - 2;
				strData.erase(strData.begin() + nEndIndex);
				iterApp = _appValues.find(strData);
				if (iterApp != _appValues.end())
				{
					LOG_ERROR << "ini文件app:" << iterApp->first << "存在两个相同的app";
				}
				else
				{
					KeyValues values;
					values.clear();
					_appValues[strData] = values;
					iterApp = _appValues.find(strData);
				}
			}
			//判断你是否是key
			else
			{
				if (iterApp == _appValues.end())
					continue;

				int eqBegin = 0;
				int eqIndex = 0;
				int eqCount = 0;
				while ((eqIndex = strData.find_first_of('=', eqBegin)) != std::string::npos)
				{
					eqCount++;
					eqBegin = eqIndex + 1;
				}

				//只能有1个等于号
				if (eqCount == 1)
				{
					eqIndex = eqBegin - 1;

					std::string key = std::string("").append(strData, 0, eqIndex);
					std::string value = std::string("").append(strData, eqIndex + 1, strData.length() - eqIndex - 1);
					iterKey = iterApp->second.find(key);
					if (iterKey != iterApp->second.end())
					{
						LOG_ERROR << "ini文件app:" << iterApp->first << "存在两个相同的Key";
					}
					else
					{
						iterApp->second[key] = value;
					}
				}
			}
		}
		
	}
}

CIniFile::~CIniFile()
{
	_appValues.clear();
}

int CIniFile::GetIntKeyValue(std::string appName, std::string keyName)
{
	AppValues::iterator iterApp = _appValues.find(appName);
	if (iterApp != _appValues.end())
	{
		KeyValues &keyVs = iterApp->second;
		KeyValues::iterator iterKey = keyVs.find(keyName);
		if (iterKey != keyVs.end())
		{
			return atoi(iterKey->second.c_str());
		}
	}

	return -1;
}

std::string CIniFile::GetStrKeyValue(std::string appName, std::string keyName)
{
	AppValues::iterator iterApp = _appValues.find(appName);
	if (iterApp != _appValues.end())
	{
		KeyValues &keyVs = iterApp->second;
		KeyValues::iterator iterKey = keyVs.find(keyName);
		if (iterKey != keyVs.end())
		{
			return iterKey->second;
		}
	}

	return std::string("");
}