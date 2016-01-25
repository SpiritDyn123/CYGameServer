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

	//�ͻ���ʹ�ô�������Encrypter��Decrypter֮ǰ����������������ӿ�
	bool SetAndGenerateRsaKey(unsigned char *publicRsaBufferAndBuffer, int nLen);

	//AES ��/����
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

	// ע��, RSA����/���ܵ����ݳ����������ƣ�����512λ��RSA��ֻ������ܼ��ܽ���64�ֽڵ����ݣ�������ʹ��32λ
	unsigned char _aesSeed[AES_KEY_CLIENT_LEN];
	RSA *_clientPrivateRsaKey;
	AES_KEY _aesEncrypterKey;
	AES_KEY _aesDecrypterKey;

	bool _running;
};

#endif