#include "../stdafx.h"
#include "COpensslEncrypter.h"

COpensslEncrypter::COpensslEncrypter()
{
	memset(_publicClientRsaKeyBuffer, 0, sizeof _publicClientRsaKeyBuffer);
	_serverPrivateRsaKey = RSA_generate_key(512, RSA_F4, NULL, NULL);
	assert(_serverPrivateRsaKey != NULL);

	unsigned char *pKey = _publicClientRsaKeyBuffer;
	_publicClientRsaKeyBuffLen = i2d_RSAPublicKey(_serverPrivateRsaKey, &pKey);
	assert(_publicClientRsaKeyBuffLen > 0);

	RSA_private_encrypt(RSA_ENCRY_LEN, aesSeed, _aesSeedRsaEncryptBuffer, _serverPrivateRsaKey, RSA_NO_PADDING);

	AES_set_encrypt_key(aesSeed, 256, &_aesEncrypterKey);
	AES_set_decrypt_key(aesSeed, 256, &_aesDecrypterKey);
}

COpensslEncrypter::~COpensslEncrypter()
{
	RSA_free(_serverPrivateRsaKey);
}

unsigned char * COpensslEncrypter::GetPublicRsaKey(int &nLen) const
{
	nLen = _publicClientRsaKeyBuffLen;
	return (unsigned char *)_publicClientRsaKeyBuffer;
}

unsigned char * COpensslEncrypter::GetPublicAesSeed(int &nLen) const
{
	nLen = RSA_ENCRY_LEN;
	return (unsigned char *)_aesSeedRsaEncryptBuffer;
}

bool COpensslEncrypter::Encrypter(const char * inBuffer,
	char * outBuffer,
	int inLen,
	int & outLen)
{
	if (inBuffer == NULL || outBuffer == NULL || inLen <= 0)
		return false;

	const unsigned char *pInBufferBegin = (const unsigned char *)inBuffer;
	unsigned char *pOutBufferBegin = (unsigned char *)outBuffer;

	outLen = 0;
	for (int i = 0; i < inLen; i += AES_ENCRY_LEN)
	{
		pInBufferBegin += outLen;
		pOutBufferBegin += outLen;
		AES_encrypt(pInBufferBegin, pOutBufferBegin, &_aesEncrypterKey);
		outLen += AES_ENCRY_LEN;
// 		if (nEndLen < AES_ENCRY_LEN)
// 		{
// 			outLen += nEndLen;
// 		}
	}

	return true;
}

bool COpensslEncrypter::Decrypter(const char * inBuffer,
	char * outBuffer,
	int inLen,
	int & outLen)
{
	if (inBuffer == NULL || outBuffer == NULL || inLen <= 0 || inLen % AES_ENCRY_LEN != 0)
		return false;

	const unsigned char *pInBufferBegin = (const unsigned char *)inBuffer;
	unsigned char *pOutBufferBegin = (unsigned char *)outBuffer;

	for (int i = 0; i < inLen; i += AES_ENCRY_LEN)
	{
		pInBufferBegin += outLen;
		pOutBufferBegin += outLen;
		AES_decrypt(pInBufferBegin, pOutBufferBegin, &_aesDecrypterKey);
		outLen += AES_ENCRY_LEN;
	}

	//assert(inLen == outLen + nEndLen);
	return true;
}
