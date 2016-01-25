#ifndef COPENSSLENCRYPTER_CLIENT_H
#define COPENSSLENCRYPTER_CLIENT_H

#include <Windows.h>
#include <time.h>
#include <iostream>
#include <assert.h>

#include "openssl/aes.h"
#include "openssl/rsa.h"

#define AES_KEY_CLIENT_LEN 32

class COpensslEncrypter_Client
{
public:
	COpensslEncrypter_Client();
	~COpensslEncrypter_Client();

	//客户端使用此类对象的Encrypter和Decrypter之前必须调用以下两个接口
	bool SetAndGenerateRsaKey(unsigned char *publicRsaBufferAndBuffer, int nLen);

	//AES 加/解密
	bool Encrypter(const char * inBuffer,
		char * outBuffer,
		int inLen,
		int & outLen);

	bool Decrypter(const char * inBuffer,
		char * outBuffer,
		int inLen,
		int & outLen);

	static const int RSA_DECRY_LEN = 64;
private:
	
	const int AES_ENCRY_LEN = 16;

	// 注意, RSA加密/解密的数据长度是有限制，例如512位的RSA就只能最多能加密解密64字节的数据，本程序使用32位
	unsigned char _aesSeed[AES_KEY_CLIENT_LEN];
	RSA *_clientPrivateRsaKey;
	AES_KEY _aesEncrypterKey;
	AES_KEY _aesDecrypterKey;

	bool _running;
};

#endif