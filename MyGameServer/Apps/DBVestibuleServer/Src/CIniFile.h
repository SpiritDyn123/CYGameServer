#ifndef CINI_FILE_H
#define  CINI_FILE_H

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>

class CIniFile
{
public:
	CIniFile(std::string fileName);

	~CIniFile();

	int GetIntKeyValue(std::string appName, std::string keyName);//Ê§°Ü·µ»Ø-1

	std::string GetStrKeyValue(std::string appName, std::string keyName);

private:
	std::string _fileName;
	std::fstream _fin;
	typedef std::map<std::string, std::string> KeyValues;
	typedef std::map<std::string, KeyValues> AppValues;
	AppValues _appValues;
};
#endif