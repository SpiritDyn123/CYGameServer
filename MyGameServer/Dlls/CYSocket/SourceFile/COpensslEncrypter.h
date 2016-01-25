#ifndef COPENSSLENCRYPTER_H
#define COPENSSLENCRYPTER_H

///////////////////////////
//服务器使用
///////////////////////////

#include <Windows.h>
#include <time.h>
#include <iostream>
#include <assert.h>

#include "openssl/aes.h"
#include "openssl/rsa.h"

#define AES_KEY_LEN 32 //产生256位的AES密匙，只能用来加/解 16字节(128位）的数据

const unsigned char aesSeed[33] = "spiritdongyananyouaresupermanyes";

class COpensslEncrypter
{
public:
	COpensslEncrypter();
	~COpensslEncrypter();


	unsigned char * GetPublicRsaKey(int &nLen) const;
	unsigned char * GetPublicAesSeed(int &nLen) const;

	//AES 加/解密
	bool Encrypter(const char * inBuffer,
		char * outBuffer,
		int inLen,
		int & outLen);

	bool Decrypter(const char * inBuffer,
		char * outBuffer,
		int inLen,
		int & outLen);

private:
	const int RSA_ENCRY_LEN = 64;
	const int AES_ENCRY_LEN = 16;

	// 注意, RSA加密/解密的数据长度是有限制，例如512位的RSA就只能最多能加密解密64字节的数据，本程序使用32位
	unsigned char _publicClientRsaKeyBuffer[512];
	int _publicClientRsaKeyBuffLen;
	unsigned char _aesSeedRsaEncryptBuffer[64];

	RSA *_serverPrivateRsaKey;
	RSA *_clientPublicRsaKey;

	AES_KEY _aesEncrypterKey;
	AES_KEY _aesDecrypterKey;

	/*RSA *_clientPublicRsaKey;*/
};

#endif