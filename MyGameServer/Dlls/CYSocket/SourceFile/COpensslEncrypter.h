#ifndef COPENSSLENCRYPTER_H
#define COPENSSLENCRYPTER_H

///////////////////////////
//������ʹ��
///////////////////////////

#include <Windows.h>
#include <time.h>
#include <iostream>
#include <assert.h>

#include "openssl/aes.h"
#include "openssl/rsa.h"

#define AES_KEY_LEN 32 //����256λ��AES�ܳף�ֻ��������/�� 16�ֽ�(128λ��������

const unsigned char aesSeed[33] = "spiritdongyananyouaresupermanyes";

class COpensslEncrypter
{
public:
	COpensslEncrypter();
	~COpensslEncrypter();


	unsigned char * GetPublicRsaKey(int &nLen) const;
	unsigned char * GetPublicAesSeed(int &nLen) const;

	//AES ��/����
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

	// ע��, RSA����/���ܵ����ݳ����������ƣ�����512λ��RSA��ֻ������ܼ��ܽ���64�ֽڵ����ݣ�������ʹ��32λ
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